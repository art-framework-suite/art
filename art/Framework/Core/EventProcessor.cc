#include "art/Framework/Core/EventProcessor.h"

#include "art/Framework/Core/Breakpoints.h"
#include "art/Framework/Services/System/ConstProductRegistry.h"
#include "art/Framework/Core/EPStates.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceFactory.h"
#include "art/Framework/Core/OccurrenceTraits.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/Schedule.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/System/CurrentModule.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/GetPassID.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "art/Version/GetReleaseVersion.h"

#include "boost/bind.hpp"
#include "boost/thread/xtime.hpp"
#include "cetlib/exception_collector.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <exception>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using boost::shared_ptr;
using fhicl::ParameterSet;

namespace art {

  namespace event_processor {

    class StateSentry
    {
    public:
      StateSentry(EventProcessor* ep) : ep_(ep), success_(false) { }
      ~StateSentry() {if(!success_) ep_->changeState(mException);}
      void succeeded() {success_ = true;}

    private:
      EventProcessor* ep_;
      bool success_;
    };
  }

  using namespace event_processor;
  using namespace art;

  namespace {

    // the next two tables must be kept in sync with the state and
    // message enums from the header

    char const* stateNames[] = {
      "Init",
      "JobReady",
      "RunGiven",
      "Running",
      "Stopping",
      "ShuttingDown",
      "Done",
      "JobEnded",
      "Error",
      "ErrorEnded",
      "End",
      "Invalid"
    };

    char const* msgNames[] = {
      "SetRun",
      "Skip",
      "RunAsync",
      "Run(ID)",
      "Run(count)",
      "BeginJob",
      "StopAsync",
      "ShutdownAsync",
      "EndJob",
      "CountComplete",
      "InputExhausted",
      "StopSignal",
      "ShutdownSignal",
      "Finished",
      "Any",
      "dtor",
      "Exception",
      "Rewind"
    };
  }
    // IMPORTANT NOTE:
    // the mAny messages are special, they must appear last in the
    // table if multiple entries for a CurrentState are present.
    // the changeState function does not use the mAny yet!!!

    struct TransEntry
    {
      State current;
      Msg   message;
      State final;
    };

    // we should use this information to initialize a two dimensional
    // table of t[CurrentState][Message] = FinalState

    /*
      the way this is current written, the async run can thread function
      can return in the "JobReady" state - but not yet cleaned up.  The
      problem is that only when stop/shutdown async is called is the
      thread cleaned up. But the stop/shudown async functions attempt
      first to change the state using messages that are not valid in
      "JobReady" state.

      I think most of the problems can be solved by using two states
      for "running": RunningS and RunningA (sync/async). The problems
      seems to be the all the transitions out of running for both
      modes of operation.  The other solution might be to only go to
      "Stopping" from Running, and use the return code from "run_p" to
      set the final state.  If this is used, then in the sync mode the
      "Stopping" state would be momentary.

     */

    TransEntry table[] = {
    // CurrentState   Message         FinalState
    // -----------------------------------------
      { sInit,          mException,      sError },
      { sInit,          mBeginJob,       sJobReady },
      { sJobReady,      mException,      sError },
      { sJobReady,      mSetRun,         sRunGiven },
      { sJobReady,      mInputRewind,    sRunning },
      { sJobReady,      mSkip,           sRunning },
      { sJobReady,      mRunID,          sRunning },
      { sJobReady,      mRunCount,       sRunning },
      { sJobReady,      mEndJob,         sJobEnded },
      { sJobReady,      mBeginJob,       sJobReady },
      { sJobReady,      mDtor,           sEnd },    // should this be allowed?

      { sJobReady,      mStopAsync,      sJobReady },
      { sJobReady,      mCountComplete,  sJobReady },
      { sJobReady,      mFinished,       sJobReady },

      { sRunGiven,      mException,      sError },
      { sRunGiven,      mRunAsync,       sRunning },
      { sRunGiven,      mBeginJob,       sRunGiven },
      { sRunGiven,      mShutdownAsync,  sShuttingDown },
      { sRunGiven,      mStopAsync,      sStopping },
      { sRunning,       mException,      sError },
      { sRunning,       mStopAsync,      sStopping },
      { sRunning,       mShutdownAsync,  sShuttingDown },
      { sRunning,       mShutdownSignal, sShuttingDown },
      { sRunning,       mCountComplete,  sStopping }, // sJobReady
      { sRunning,       mInputExhausted, sStopping }, // sJobReady

      { sStopping,      mInputRewind,    sRunning }, // The looper needs this
      { sStopping,      mException,      sError },
      { sStopping,      mFinished,       sJobReady },
      { sStopping,      mCountComplete,  sJobReady },
      { sStopping,      mShutdownSignal, sShuttingDown },
      { sStopping,      mStopAsync,      sStopping },     // stay
      { sStopping,      mInputExhausted, sStopping },     // stay
      //{ sStopping,      mAny,            sJobReady },     // <- ??????
      { sShuttingDown,  mException,      sError },
      { sShuttingDown,  mShutdownSignal, sShuttingDown },
      { sShuttingDown,  mCountComplete,  sDone }, // needed?
      { sShuttingDown,  mInputExhausted, sDone }, // needed?
      { sShuttingDown,  mFinished,       sDone },
      //{ sShuttingDown,  mShutdownAsync,  sShuttingDown }, // only one at
      //{ sShuttingDown,  mStopAsync,      sShuttingDown }, // a time
      //{ sShuttingDown,  mAny,            sDone },         // <- ??????
      { sDone,          mEndJob,         sJobEnded },
      { sDone,          mException,      sError },
      { sJobEnded,      mDtor,           sEnd },
      { sJobEnded,      mException,      sError },
      { sError,         mEndJob,         sError },   // funny one here
      { sError,         mDtor,           sError },   // funny one here
      { sInit,          mDtor,           sEnd },     // for StorM dummy EP
      { sStopping,      mShutdownAsync,  sShuttingDown }, // For FUEP tests
      { sInvalid,       mAny,            sInvalid }
    };


    // Note: many of the messages generate the mBeginJob message first
    //  mRunID, mRunCount, mSetRun

  // ---------------------------------------------------------------
  shared_ptr<InputSource>
  makeInput(ParameterSet const& params,
            std::string const &processName,
            ProductRegistry& preg,
            boost::shared_ptr<ActivityRegistry> areg) {

     ParameterSet defaultEmptySource;
     defaultEmptySource.put("module_type", "EmptyEvent");
     defaultEmptySource.put("module_label", "source");
     defaultEmptySource.put("maxEvents", 1);

     // find single source
     bool sourceSpecified = false;
     ParameterSet main_input = defaultEmptySource;
     try {
        try {
           main_input = params.get<fhicl::ParameterSet>("source");
        }
        // TODO: catch correct exception.
        catch (...) {
           // TODO: inform that we're using the default source configuration.
        }
        // Fill in "ModuleDescription", in case the input source produces
        // any EDproducts,which would be registered in the ProductRegistry.
        // Also fill in the process history item for this process.

        ModuleDescription md;
        md.parameterSetID_ = main_input.id();
        md.moduleName_ = main_input.get<std::string>("module_type");
        md.moduleLabel_ = main_input.get<std::string>("module_label");

        md.processConfiguration_ = ProcessConfiguration(processName,
                                                        params.id(),
                                                        getReleaseVersion(), getPassID());

        sourceSpecified = true;
        InputSourceDescription isdesc(md, preg, areg,
                                      main_input.get<int>("maxEvents", -1),
                                      main_input.get<int>("maxSubRuns", -1));
        areg->preSourceConstructionSignal_(md);

        shared_ptr<InputSource> input(InputSourceFactory::makeInputSource(main_input,
                                                                          isdesc).release());
        areg->postSourceConstructionSignal_(md);

        return input;
     }
     catch(art::Exception const& iException) {
        if(sourceSpecified == false &&
           errors::Configuration == iException.categoryCode()) {
           throw art::Exception(errors::Configuration, "FailedInputSource")
              << "Configuration of main input source has failed\n"
              << iException;
        } else {
           throw;
        }
     }
     return shared_ptr<InputSource>();
  }

  // -------- functions to help prepare the services for initialization --------

   typedef std::vector<ParameterSet> ParameterSets;

   void addService(std::string const& name, ParameterSets& service_set)
  {
    service_set.push_back(ParameterSet());
    service_set.back().put("service_type",name);
  }

   void addOptionalService(std::string const& name,
                          ParameterSet const& source,
                          ParameterSets& service_set)
  {
    try {
      service_set.push_back(source.get<ParameterSet>(name));
      service_set.back().put("service_type",name);
    }
    catch(fhicl::exception&)
      {
        // ignore
      }
  }

   void addService(std::string const& name, ParameterSet const& source, ParameterSets& service_set)
  {
    service_set.push_back(source.get<ParameterSet>(name,ParameterSet()));
    service_set.back().put("service_type",name);
  }

  void extractServices(ParameterSet const& services, ParameterSets& service_set)
  {
    // this is not ideal.  Need to change the ServiceRegistry "createSet" and ServicesManager "put"
    // functions to take the parameter set vector and a list of service objects to be added to
    // the service token.  Alternatively we could get the service token and be allowed to add
    // service objects to it.  Since the servicetoken contains the servicemanager, we might
    // be able to simply add a function to the serviceregistry or servicesmanager that given
    // a service token, it injects a new service object using the "put" of the
    // servicesManager.
    // order might be important here

    // only configured if pset present in services
    addOptionalService("SimpleMemoryCheck",services, service_set);
    addOptionalService("Timing",services,service_set);
    addOptionalService("TFileService",services,service_set);

    ParameterSet user_services = services.get<ParameterSet>("user",ParameterSet());

    std::vector<std::string> keys = user_services.get_pset_keys();

    for(std::vector<std::string>::iterator i=keys.begin(),e=keys.end();i!=e;++i)
      addService(*i, user_services,service_set);
  }

  // ----------- event processor functions ------------------

  EventProcessor::EventProcessor(ParameterSet const& pset):
    preProcessEventSignal_(),
    postProcessEventSignal_(),
    actReg_(new ActivityRegistry),
    wreg_(actReg_),
    preg_(),
    serviceToken_(),
    input_(),
    schedule_(),
    act_table_(),
    state_(sInit),
    event_loop_(),
    state_lock_(),
    stop_lock_(),
    stopper_(),
    stop_count_(-1),
    last_rc_(epSuccess),
    last_error_text_(),
    id_set_(false),
    event_loop_id_(),
    my_sig_num_(getSigNum()),
    fb_(),
    shouldWeStop_(false),
    alreadyHandlingException_(false)
  {
    // The BranchIDListRegistry and ProductIDListRegistry are indexed registries, and are singletons.
    //  They must be cleared here because some processes run multiple EventProcessors in succession.
    BranchIDListHelper::clearRegistries();

    // TODO: Fix const-correctness. The ParameterSets that are
    // returned here should be const, so that we can be sure they are
    // not modified.

    ParameterSet services = pset.get<ParameterSet>("services",ParameterSet());
    ParameterSet scheduler = services.get<ParameterSet>("scheduler", ParameterSet());

    ParameterSet fpc_pset = services.get<ParameterSet>("floating_point_control",ParameterSet());

    fileMode_ = scheduler.get<std::string>("fileMode", "");
    handleEmptyRuns_ = scheduler.get<bool>("handleEmptyRuns", true);
    handleEmptySubRuns_ = scheduler.get<bool>("handleEmptySubRuns", true);
    bool wantTracer = scheduler.get<bool>("wantTracer",false);
    std::string processName = pset.get<std::string>("process_name");

    // build a list of service parameter sets that will be used by the service registry
    ParameterSets service_set;
    extractServices(services,service_set);

    // configured based on optional parameters
    if(wantTracer) addService("Tracer",service_set);

    serviceToken_ = ServiceRegistry::createSet(service_set);

    // NOTE: the order here might be backwards, due to the "push_front" registering
    // that sigc++ does way in the guts of the add operation.

    typedef art::TriggerNamesService TNS;
    typedef ConstProductRegistry CPR;
    // no configuration available
    serviceToken_.add(std::auto_ptr<CurrentModule>(new CurrentModule(*actReg_)));
    // special construction
    serviceToken_.add(std::auto_ptr<CPR>(new CPR(preg_)));
    serviceToken_.add(std::auto_ptr<TNS>(new TNS(pset)));
    serviceToken_.add(std::auto_ptr<FloatingPointControl>(new FloatingPointControl(fpc_pset,*actReg_)));
    // May be able to do this with CurrentModule serive, else might need
    // small service to configure the ContextProvider of the
    // MessageFacility.
    // serviceToken_.add(std::auto_ptr<MessageLogger>(new
    // MessageLogger(logger_pset,*actReg_)));

    serviceToken_.forceCreation(*actReg_);

    //make the services available
    ServiceRegistry::Operate operate(serviceToken_);

    act_table_ = ActionTable(pset);
    input_= makeInput(pset, processName,preg_, actReg_);

    schedule_ = std::auto_ptr<Schedule>
      (new Schedule(pset,
                    ServiceRegistry::instance().get<TNS>(),
                    wreg_,
                    preg_,
                    act_table_,
                    actReg_));

    //   initialize(token,legacy);
    FDEBUG(2) << pset.to_string() << std::endl;

    connectSigs(this);
    BranchIDListHelper::updateRegistries(preg_);
  }

  EventProcessor::~EventProcessor()
  {
    // Make the services available while everything is being deleted.
    ServiceToken token = getToken();
    ServiceRegistry::Operate op(token);

    // The state machine should have already been cleaned up
    // and destroyed at this point by a call to EndJob or
    // earlier when it completed processing events, but if it
    // has not been we'll take care of it here at the last moment.
    // This could cause problems if we are already handling an
    // exception and another one is thrown here ..  For a critical
    // executable the solution to this problem is for the code using
    // the EventProcessor to explicitly call EndJob or use runToCompletion,
    // then the next line of code is never executed.
    terminateMachine();

    try {
      changeState(mDtor);
    }
    catch(cet::exception& e)
      {
        mf::LogError("System") << e.explain_self() << "\n";
      }

    // manually destroy all these thing that may need the services around
    schedule_.reset();
    input_.reset();
    wreg_.clear();
    actReg_.reset();
  }

  void
  EventProcessor::rewind()
  {
    beginJob(); //make sure this was called
    changeState(mStopAsync);
    changeState(mInputRewind);
    {
      StateSentry toerror(this);

      //make the services available
      ServiceRegistry::Operate operate(serviceToken_);

      {
        input_->repeat();
        input_->rewind();
      }
      changeState(mCountComplete);
      toerror.succeeded();
    }
    changeState(mFinished);
  }

  std::auto_ptr<EventPrincipal>
  EventProcessor::doOneEvent(EventID const& id) {
    std::auto_ptr<EventPrincipal> pep;
    {
      pep = input_->readEvent(id);
    }
    procOneEvent(pep.get());
    return pep;
  }

  void
  EventProcessor::procOneEvent(EventPrincipal *pep) {
    if(0 != pep) {
      schedule_->processOneOccurrence<OccurrenceTraits<EventPrincipal, BranchActionBegin> >(*pep);
    }
  }

  EventProcessor::StatusCode
  EventProcessor::run(EventID const& id)
  {
    beginJob(); //make sure this was called
    changeState(mRunID);
    StateSentry toerror(this);
    Status rc = epSuccess;

    //make the services available
    ServiceRegistry::Operate operate(serviceToken_);

    if(doOneEvent(id).get() == 0) {
      changeState(mInputExhausted);
    } else {
      changeState(mCountComplete);
      rc = epInputComplete;
    }
    toerror.succeeded();
    changeState(mFinished);
    return rc;
  }

  EventProcessor::StatusCode
  EventProcessor::skip(int numberToSkip)
  {
    beginJob(); //make sure this was called
    changeState(mSkip);
    {
      StateSentry toerror(this);

      //make the services available
      ServiceRegistry::Operate operate(serviceToken_);

      {
        input_->skipEvents(numberToSkip);
      }
      changeState(mCountComplete);
      toerror.succeeded();
    }
    changeState(mFinished);
    return epSuccess;
  }

  void
  EventProcessor::beginJob()
  {
    if(state_ != sInit) return;
    bk::beginJob();
    // can only be run if in the initial state
    changeState(mBeginJob);

    // StateSentry toerror(this); // should we add this ?
    //make the services available
    ServiceRegistry::Operate operate(serviceToken_);

    //NOTE:  This implementation assumes 'Job' means one call
    // the EventProcessor::run
    // If it really means once per 'application' then this code will
    // have to be changed.
    // Also have to deal with case where have 'run' then new Module
    // added and do 'run'
    // again.  In that case the newly added Module needs its 'beginJob'
    // to be called.
    try {
      input_->doBeginJob();
    } catch(cet::exception& e) {
      mf::LogError("BeginJob") << "A cet::exception happened while processing"
                                  " the beginJob of the 'source'\n";
      e << "A cet::exception happened while processing"
           " the beginJob of the 'source'\n";
      throw;
    } catch(std::exception& e) {
      mf::LogError("BeginJob") << "A std::exception happened while processing"
                                  " the beginJob of the 'source'\n";
      throw;
    } catch(...) {
      mf::LogError("BeginJob") << "An unknown exception happened while"
                                  " processing the beginJob of the 'source'\n";
      throw;
    }
    schedule_->beginJob();
    actReg_->postBeginJobSignal_();

    Schedule::Workers aw_vec;
    schedule_->getAllWorkers(aw_vec);
    actReg_->postBeginJobWorkersSignal_(input_.get(), aw_vec);

    // toerror.succeeded(); // should we add this?
  }

  void
  EventProcessor::endJob()
  {
    // Collects exceptions, so we don't throw before all operations are performed.
    cet::exception_collector c;

    // only allowed to run if state is sIdle,sJobReady,sRunGiven
    c.call(boost::bind(&EventProcessor::changeState, this, mEndJob));

    //make the services available
    ServiceRegistry::Operate operate(serviceToken_);

    c.call(boost::bind(&EventProcessor::terminateMachine, this));
    c.call(boost::bind(&Schedule::endJob, schedule_.get()));
    c.call(boost::bind(&InputSource::doEndJob, input_));
    c.call(boost::bind(&ActivityRegistry::PostEndJob::operator(), &actReg_->postEndJobSignal_));
  }

  ServiceToken
  EventProcessor::getToken()
  {
    return serviceToken_;
  }

  void
  EventProcessor::connectSigs(EventProcessor* ep)
  {
    // When the FwkImpl signals are given, pass them to the
    // appropriate EventProcessor signals so that the outside world
    // can see the signal.
    actReg_->preProcessEventSignal_.connect(ep->preProcessEventSignal_);
    actReg_->postProcessEventSignal_.connect(ep->postProcessEventSignal_);
  }

  std::vector<ModuleDescription const*>
  EventProcessor::getAllModuleDescriptions() const
  {
    return schedule_->getAllModuleDescriptions();
  }

  int
  EventProcessor::totalEvents() const
  {
    return schedule_->totalEvents();
  }

  int
  EventProcessor::totalEventsPassed() const
  {
    return schedule_->totalEventsPassed();
  }

  int
  EventProcessor::totalEventsFailed() const
  {
    return schedule_->totalEventsFailed();
  }

  void
  EventProcessor::enableEndPaths(bool active)
  {
    schedule_->enableEndPaths(active);
  }

  bool
  EventProcessor::endPathsEnabled() const
  {
    return schedule_->endPathsEnabled();
  }

  void
  EventProcessor::getTriggerReport(TriggerReport& rep) const
  {
    schedule_->getTriggerReport(rep);
  }

  void
  EventProcessor::clearCounters()
  {
    schedule_->clearCounters();
  }


  char const* EventProcessor::currentStateName() const
  {
    return stateName(getState());
  }

  char const* EventProcessor::stateName(State s) const
  {
    return stateNames[s];
  }

  char const* EventProcessor::msgName(Msg m) const
  {
    return msgNames[m];
  }

  State EventProcessor::getState() const
  {
    return state_;
  }

  EventProcessor::StatusCode EventProcessor::statusAsync() const
  {
    // the thread will record exception/error status in the event processor
    // for us to look at and report here
    return last_rc_;
  }

  EventProcessor::StatusCode
  EventProcessor::waitForAsyncCompletion(unsigned int timeout_seconds)
  {
    bool rc = true;
    boost::xtime timeout;
    boost::xtime_get(&timeout, boost::TIME_UTC);
    timeout.sec += timeout_seconds;

    // make sure to include a timeout here so we don't wait forever
    // I suspect there are still timing issues with thread startup
    // and the setting of the various control variables (stop_count,id_set)
    {
      boost::mutex::scoped_lock sl(stop_lock_);

      // look here - if runAsync not active, just return the last return code
      if(stop_count_ < 0) return last_rc_;

      if(timeout_seconds==0)
        while(stop_count_==0) stopper_.wait(sl);
      else
        while(stop_count_==0 &&
              (rc = stopper_.timed_wait(sl,timeout)) == true);

      if(rc == false)
        {
          // timeout occurred
          // if(id_set_) pthread_kill(event_loop_id_,my_sig_num_);
          // this is a temporary hack until we get the input source
          // upgraded to allow blocking input sources to be unblocked

          // the next line is dangerous and causes all sorts of trouble
          if(id_set_) pthread_cancel(event_loop_id_);

          // we will not do anything yet
          mf::LogWarning("timeout")
            << "An asynchronous request was made to shut down the event loop"
               " and the event loop did not shutdown after "
            << timeout_seconds << " seconds\n";
        }
      else
        {
          event_loop_->join();
          event_loop_.reset();
          id_set_ = false;
          stop_count_ = -1;
        }
    }
    return rc==false?epTimedOut:last_rc_;
  }

  EventProcessor::StatusCode
  EventProcessor::waitTillDoneAsync(unsigned int timeout_value_secs)
  {
    StatusCode rc = waitForAsyncCompletion(timeout_value_secs);
    if(rc!=epTimedOut) changeState(mCountComplete);
    else errorState();
    return rc;
  }


  EventProcessor::StatusCode EventProcessor::stopAsync(unsigned int secs)
  {
    changeState(mStopAsync);
    StatusCode rc = waitForAsyncCompletion(secs);
    if(rc!=epTimedOut) changeState(mFinished);
    else errorState();
    return rc;
  }

  EventProcessor::StatusCode EventProcessor::shutdownAsync(unsigned int secs)
  {
    changeState(mShutdownAsync);
    StatusCode rc = waitForAsyncCompletion(secs);
    if(rc!=epTimedOut) changeState(mFinished);
    else errorState();
    return rc;
  }

  void EventProcessor::errorState()
  {
    state_ = sError;
  }

  // next function irrelevant now
  EventProcessor::StatusCode EventProcessor::doneAsync(Msg m)
  {
    // make sure to include a timeout here so we don't wait forever
    // I suspect there are still timing issues with thread startup
    // and the setting of the various control variables (stop_count,id_set)
    changeState(m);
    return waitForAsyncCompletion(60*2);
  }

  void EventProcessor::changeState(Msg msg)
  {
    // most likely need to serialize access to this routine

    boost::mutex::scoped_lock sl(state_lock_);
    State curr = state_;
    int rc;
    // found if(not end of table) and
    // (state == table.state && (msg == table.message || msg == any))
    for(rc = 0;
        table[rc].current != sInvalid &&
          (curr != table[rc].current ||
           (curr == table[rc].current &&
             msg != table[rc].message && table[rc].message != mAny));
        ++rc);

    if(table[rc].current == sInvalid)
      throw cet::exception("BadState")
        << "A member function of EventProcessor has been called in an"
        << " inappropriate order.\n"
        << "Bad transition from " << stateName(curr) << " "
        << "using message " << msgName(msg) << "\n"
        << "No where to go from here.\n";

    FDEBUG(1) << "changeState: current=" << stateName(curr)
              << ", message=" << msgName(msg)
              << " -> new=" << stateName(table[rc].final) << "\n";

    state_ = table[rc].final;
  }

  void EventProcessor::runAsync()
  {
    using boost::thread;
    beginJob();
    {
      boost::mutex::scoped_lock sl(stop_lock_);
      if(id_set_==true) {
          std::string err("runAsync called while async event loop already running\n");
          mf::LogError("FwkJob") << err;
          throw cet::exception("BadState") << err;
      }

      changeState(mRunAsync);

      stop_count_=0;
      last_rc_=epSuccess; // forget the last value!
      event_loop_.reset(new thread(boost::bind(EventProcessor::asyncRun,this)));
      boost::xtime timeout;
      boost::xtime_get(&timeout, boost::TIME_UTC);
      timeout.sec += 60; // 60 seconds to start!!!!
      if(starter_.timed_wait(sl,timeout)==false) {
          // yikes - the thread did not start
          throw cet::exception("BadState")
            << "Async run thread did not start in 60 seconds\n";
      }
    }
  }

  void EventProcessor::asyncRun(EventProcessor* me)
  {
    // set up signals to allow for interruptions
    // ignore all other signals
    // make sure no exceptions escape out

    // temporary hack until we modify the input source to allow
    // wakeup calls from other threads.  This mimics the solution
    // in EventFilter/Processor, which I do not like.
    // allowing cancels means that the thread just disappears at
    // certain points.  This is bad for C++ stack variables.
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,0);
    //pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,0);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,0);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,0);

    {
      boost::mutex::scoped_lock(me->stop_lock_);
      me->event_loop_id_ = pthread_self();
      me->id_set_ = true;
      me->starter_.notify_all();
    }

    Status rc = epException;
    FDEBUG(2) << "asyncRun starting ...........\n";

    try {
      bool onlineStateTransitions = true;
      rc = me->runToCompletion(onlineStateTransitions);
    }
    catch (cet::exception& e) {
      mf::LogError("FwkJob") << "cet::exception caught in "
                                "EventProcessor::asyncRun\n"
                             << e.explain_self();
      me->last_error_text_ = e.explain_self();
    }
    catch (std::exception& e) {
      mf::LogError("FwkJob") << "Standard library exception caught in "
                                "EventProcessor::asyncRun\n"
                             << e.what();
      me->last_error_text_ = e.what();
    }
    catch (...) {
      mf::LogError("FwkJob") << "Unknown exception caught in "
                                "EventProcessor::asyncRun\n";
      me->last_error_text_ = "Unknown exception caught";
      rc = epOther;
    }

    me->last_rc_ = rc;

    {
      // notify anyone waiting for exit that we are doing so now
      boost::mutex::scoped_lock sl(me->stop_lock_);
      ++me->stop_count_;
      me->stopper_.notify_all();
    }
    FDEBUG(2) << "asyncRun ending ...........\n";
  }


  art::EventProcessor::StatusCode
  EventProcessor::runToCompletion(bool onlineStateTransitions) {

    StateSentry toerror(this);

    int numberOfEventsToProcess = -1;
    StatusCode returnCode = runCommon(onlineStateTransitions, numberOfEventsToProcess);

    if (machine_.get() != 0) {
      throw art::Exception(errors::LogicError)
        << "State machine not destroyed on exit from EventProcessor::runToCompletion\n"
        << "Please report this error to the Framework group\n";
    }

    toerror.succeeded();

    return returnCode;
  }

  art::EventProcessor::StatusCode
  EventProcessor::runEventCount(int numberOfEventsToProcess) {

    StateSentry toerror(this);

    bool onlineStateTransitions = false;
    StatusCode returnCode = runCommon(onlineStateTransitions, numberOfEventsToProcess);

    toerror.succeeded();

    return returnCode;
  }

  art::EventProcessor::StatusCode
  EventProcessor::runCommon(bool onlineStateTransitions, int numberOfEventsToProcess) {

    beginJob(); //make sure this was called

    if (!onlineStateTransitions) changeState(mRunCount);

    StatusCode returnCode = epSuccess;
    stateMachineWasInErrorState_ = false;

    // make the services available
    ServiceRegistry::Operate operate(serviceToken_);

    if (machine_.get() == 0) {

      statemachine::FileMode fileMode;
      if (fileMode_.empty()) fileMode = statemachine::FULLMERGE;
      else if (fileMode_ == std::string("MERGE")) fileMode = statemachine::MERGE;
      else if (fileMode_ == std::string("NOMERGE")) fileMode = statemachine::NOMERGE;
      else if (fileMode_ == std::string("FULLMERGE")) fileMode = statemachine::FULLMERGE;
      else if (fileMode_ == std::string("FULLLUMIMERGE")) fileMode = statemachine::FULLLUMIMERGE;
      else {
        throw art::Exception(errors::Configuration, "Illegal fileMode parameter value: ")
            << fileMode_ << ".\n"
            << "Legal values are 'MERGE', 'NOMERGE', 'FULLMERGE', and 'FULLLUMIMERGE'.\n";
      }

      machine_.reset(new statemachine::Machine(this,
                                               fileMode,
                                               handleEmptyRuns_,
                                               handleEmptySubRuns_));

      machine_->initiate();
    }

    try {

      input::ItemType itemType;

      int iEvents = 0;

      while (true) {

        itemType = input_->nextItemType();

        FDEBUG(1) << "itemType = " << itemType << "\n";

        // These are used for asynchronous running only and
        // and are checking to see if stopAsync or shutdownAsync
        // were called from another thread.  In the future, we
        // may need to do something better than polling the state.
        // With the current code this is the simplest thing and
        // it should always work.  If the interaction between
        // threads becomes more complex this may cause problems.
        if (state_ == sStopping) {
          FDEBUG(1) << "In main processing loop, encountered sStopping state\n";
          machine_->process_event(statemachine::Stop());
          break;
        }
        else if (state_ == sShuttingDown) {
          FDEBUG(1) << "In main processing loop, encountered sShuttingDown state\n";
          machine_->process_event(statemachine::Stop());
          break;
        }

        // Look for a shutdown signal
        {
          boost::mutex::scoped_lock sl(usr2_lock);
          if (art::shutdown_flag) {
            changeState(mShutdownSignal);
            returnCode = epSignal;
            machine_->process_event(statemachine::Stop());
            break;
          }
        }

        if (itemType == input::IsStop) {
          machine_->process_event(statemachine::Stop());
        }
        else if (itemType == input::IsFile) {
          machine_->process_event(statemachine::File());
        }
        else if (itemType == input::IsRun) {
          machine_->process_event(statemachine::Run(input_->run()));
        }
        else if (itemType == input::IsSubRun) {
          machine_->process_event(statemachine::SubRun(input_->subRun()));
        }
        else if (itemType == input::IsEvent) {
          machine_->process_event(statemachine::Event());
          ++iEvents;
          if (numberOfEventsToProcess > 0 && iEvents >= numberOfEventsToProcess) {
            returnCode = epCountComplete;
            changeState(mInputExhausted);
            FDEBUG(1) << "Event count complete, pausing event loop\n";
            break;
          }
        }
        // This should be impossible
        else {
          throw art::Exception(errors::LogicError)
            << "Unknown next item type passed to EventProcessor\n"
            << "Please report this error to the Framework group\n";
        }

        if (machine_->terminated()) {
          changeState(mInputExhausted);
          break;
        }
      }  // End of loop over state machine events
    } // Try block

    // Some comments on exception handling related to the boost state machine:
    //
    // Some states used in the machine are special because they
    // perform actions while the machine is being terminated, actions
    // such as close files, call endRun, call endSubRun etc ..  Each of these
    // states has two functions that perform these actions.  The functions
    // are almost identical.  The major difference is that one version
    // catches all exceptions and the other lets exceptions pass through.
    // The destructor catches them and the other function named "exit" lets
    // them pass through.  On a normal termination, boost will always call
    // "exit" and then the state destructor.  In our state classes, the
    // the destructors do nothing if the exit function already took
    // care of things.  Here's the interesting part.  When boost is
    // handling an exception the "exit" function is not called (a boost
    // feature).
    //
    // If an exception occurs while the boost machine is in control
    // (which usually means inside a process_event call), then
    // the boost state machine destroys its states and "terminates" itself.
    // This already done before we hit the catch blocks below. In this case
    // the call to terminateMachine below only destroys an already
    // terminated state machine.  Because exit is not called, the state destructors
    // handle cleaning up subRuns, runs, and files.  The destructors swallow
    // all exceptions and only pass through the exceptions messages which
    // are tacked onto the original exception below.
    //
    // If an exception occurs when the boost state machine is not
    // in control (outside the process_event functions), then boost
    // cannot destroy its own states.  The terminateMachine function
    // below takes care of that.  The flag "alreadyHandlingException"
    // is set true so that the state exit functions do nothing (and
    // cannot throw more exceptions while handling the first).  Then the
    // state destructors take care of this because exit did nothing.
    //
    // In both cases above, the EventProcessor::endOfLoop function is
    // not called because it can throw exceptions.
    //
    // One tricky aspect of the state machine is that things which can
    // throw should not be invoked by the state machine while another
    // exception is being handled.
    // Another tricky aspect is that it appears to be important to
    // terminate the state machine before invoking its destructor.
    // We've seen crashes which are not understood when that is not
    // done.  Maintainers of this code should be careful about this.

    catch (cet::exception& e) {
      alreadyHandlingException_ = true;
      terminateMachine();
      alreadyHandlingException_ = false;
      e << "cet::exception caught in EventProcessor and rethrown\n";
      e << exceptionMessageSubRuns_;
      e << exceptionMessageRuns_;
      e << exceptionMessageFiles_;
      throw e;
    }
    catch (std::bad_alloc& e) {
      alreadyHandlingException_ = true;
      terminateMachine();
      alreadyHandlingException_ = false;
      throw cet::exception("std::bad_alloc")
        << "The EventProcessor caught a std::bad_alloc exception and converted it to a cet::exception\n"
        << "The job has probably exhausted the virtual memory available to the process.\n"
        << exceptionMessageSubRuns_
        << exceptionMessageRuns_
        << exceptionMessageFiles_;
    }
    catch (std::exception& e) {
      alreadyHandlingException_ = true;
      terminateMachine();
      alreadyHandlingException_ = false;
      throw cet::exception("StdException")
        << "The EventProcessor caught a std::exception and converted it to a cet::exception\n"
        << "Previous information:\n" << e.what() << "\n"
        << exceptionMessageSubRuns_
        << exceptionMessageRuns_
        << exceptionMessageFiles_;
    }
    catch (...) {
      alreadyHandlingException_ = true;
      terminateMachine();
      alreadyHandlingException_ = false;
      throw cet::exception("Unknown")
        << "The EventProcessor caught an unknown exception type and converted it to a cet::exception\n"
        << exceptionMessageSubRuns_
        << exceptionMessageRuns_
        << exceptionMessageFiles_;
    }

    if (machine_->terminated()) {
      FDEBUG(1) << "The state machine reports it has been terminated\n";
      machine_.reset();
    }

    if (!onlineStateTransitions) changeState(mFinished);

    if (stateMachineWasInErrorState_) {
      throw cet::exception("BadState")
        << "The boost state machine in the EventProcessor exited after\n"
        << "entering the Error state.\n";
    }

    return returnCode;
  }

  void EventProcessor::readFile() {
    FDEBUG(1) << " \treadFile\n";
    fb_ = input_->readFile();
  }

  void EventProcessor::closeInputFile() {
    input_->closeFile();
    FDEBUG(1) << "\tcloseInputFile\n";
  }

  void EventProcessor::openOutputFiles() {
    schedule_->openOutputFiles(*fb_);
    FDEBUG(1) << "\topenOutputFiles\n";
  }

  void EventProcessor::closeOutputFiles() {
    schedule_->closeOutputFiles();
    FDEBUG(1) << "\tcloseOutputFiles\n";
  }

  void EventProcessor::respondToOpenInputFile() {
    schedule_->respondToOpenInputFile(*fb_);
    FDEBUG(1) << "\trespondToOpenInputFile\n";
  }

  void EventProcessor::respondToCloseInputFile() {
    schedule_->respondToCloseInputFile(*fb_);
    FDEBUG(1) << "\trespondToCloseInputFile\n";
  }

  void EventProcessor::respondToOpenOutputFiles() {
    schedule_->respondToOpenOutputFiles(*fb_);
    FDEBUG(1) << "\trespondToOpenOutputFiles\n";
  }

  void EventProcessor::respondToCloseOutputFiles() {
    schedule_->respondToCloseOutputFiles(*fb_);
    FDEBUG(1) << "\trespondToCloseOutputFiles\n";
  }

  void EventProcessor::startingNewLoop() {
    shouldWeStop_ = false;
    FDEBUG(1) << "\tstartingNewLoop\n";
  }

  bool EventProcessor::endOfLoop() {
    FDEBUG(1) << "\tendOfLoop\n";
    return true;
  }

  void EventProcessor::rewindInput() {
    input_->repeat();
    input_->rewind();
    FDEBUG(1) << "\trewind\n";
  }

  void EventProcessor::prepareForNextLoop() {
    FDEBUG(1) << "\tprepareForNextLoop\n";
  }

  void EventProcessor::writeSubRunCache() {
    while (!principalCache_.noMoreSubRuns()) {
      schedule_->writeSubRun(principalCache_.lowestSubRun());
      principalCache_.deleteLowestSubRun();
    }
    FDEBUG(1) << "\twriteSubRunCache\n";
  }

  void EventProcessor::writeRunCache() {
    while (!principalCache_.noMoreRuns()) {
      schedule_->writeRun(principalCache_.lowestRun());
      principalCache_.deleteLowestRun();
    }
    FDEBUG(1) << "\twriteRunCache\n";
  }

  bool EventProcessor::shouldWeCloseOutput() const {
    FDEBUG(1) << "\tshouldWeCloseOutput\n";
    return schedule_->shouldWeCloseOutput();
  }

  void EventProcessor::doErrorStuff() {
    FDEBUG(1) << "\tdoErrorStuff\n";
    mf::LogError("StateMachine")
      << "The EventProcessor state machine encountered an unexpected event\n"
         "and went to the error state\n"
         "Will attempt to terminate processing normally\n"
         "This likely indicates a bug in an input module, corrupted input, or both\n";
    stateMachineWasInErrorState_ = true;
  }

  void EventProcessor::beginRun(int run) {
    RunPrincipal& runPrincipal = principalCache_.runPrincipal(run);
    schedule_->processOneOccurrence<OccurrenceTraits<RunPrincipal, BranchActionBegin> >(runPrincipal);
    FDEBUG(1) << "\tbeginRun " << run << "\n";
  }

  void EventProcessor::endRun(int run) {
    RunPrincipal& runPrincipal = principalCache_.runPrincipal(run);
    input_->doEndRun(runPrincipal);
    schedule_->processOneOccurrence<OccurrenceTraits<RunPrincipal, BranchActionEnd> >(runPrincipal);
    FDEBUG(1) << "\tendRun " << run << "\n";
  }

  void EventProcessor::beginSubRun(int run, int subRun) {
    SubRunPrincipal& subRunPrincipal = principalCache_.subRunPrincipal(run, subRun);
    // NOTE: Using 0 as the event number for the begin of a subRun block is a bad idea
    // subRun blocks know their start and end times why not also start and end events?
    schedule_->processOneOccurrence<OccurrenceTraits<SubRunPrincipal, BranchActionBegin> >(subRunPrincipal);
    FDEBUG(1) << "\tbeginSubRun " << run << "/" << subRun << "\n";
  }

  void EventProcessor::endSubRun(int run, int subRun) {
    SubRunPrincipal& subRunPrincipal = principalCache_.subRunPrincipal(run, subRun);
    input_->doEndSubRun(subRunPrincipal);
    //NOTE: Using the max event number for the end of a subRun block is a bad idea
    // subRun blocks know their start and end times why not also start and end events?
    schedule_->processOneOccurrence<OccurrenceTraits<SubRunPrincipal, BranchActionEnd> >(subRunPrincipal);
    FDEBUG(1) << "\tendSubRun " << run << "/" << subRun << "\n";
  }

  int EventProcessor::readAndCacheRun() {
    principalCache_.insert(input_->readRun());
    FDEBUG(1) << "\treadAndCacheRun " << "\n";
    return principalCache_.runPrincipal().run();
  }

  int EventProcessor::readAndCacheSubRun() {
    principalCache_.insert(input_->readSubRun(principalCache_.runPrincipalPtr()));
    FDEBUG(1) << "\treadAndCacheSubRun " << "\n";
    return principalCache_.subRunPrincipal().subRun();
  }

  void EventProcessor::writeRun(int run) {
    schedule_->writeRun(principalCache_.runPrincipal(run));
    FDEBUG(1) << "\twriteRun " << run << "\n";
  }

  void EventProcessor::deleteRunFromCache(int run) {
    principalCache_.deleteRun(run);
    FDEBUG(1) << "\tdeleteRunFromCache " << run << "\n";
  }

  void EventProcessor::writeSubRun(int run, int subRun) {
    schedule_->writeSubRun(principalCache_.subRunPrincipal(run, subRun));
    FDEBUG(1) << "\twriteSubRun " << run << "/" << subRun << "\n";
  }

  void EventProcessor::deleteSubRunFromCache(int run, int subRun) {
    principalCache_.deleteSubRun(run, subRun);
    FDEBUG(1) << "\tdeleteSubRunFromCache " << run << "/" << subRun << "\n";
  }

  void EventProcessor::readEvent() {
    sm_evp_ = input_->readEvent(principalCache_.subRunPrincipalPtr());
    FDEBUG(1) << "\treadEvent\n";
  }

  void EventProcessor::processEvent() {
    schedule_->processOneOccurrence<OccurrenceTraits<EventPrincipal, BranchActionBegin> >(*sm_evp_);

    FDEBUG(1) << "\tprocessEvent\n";
  }

  bool EventProcessor::shouldWeStop() const {
    FDEBUG(1) << "\tshouldWeStop\n";
    if (shouldWeStop_) return true;
    return schedule_->terminate();
  }

  void EventProcessor::setExceptionMessageFiles(std::string& message) {
    exceptionMessageFiles_ = message;
  }

  void EventProcessor::setExceptionMessageRuns(std::string& message) {
    exceptionMessageRuns_ = message;
  }

  void EventProcessor::setExceptionMessageSubRuns(std::string& message) {
    exceptionMessageSubRuns_ = message;
  }

  bool EventProcessor::alreadyHandlingException() const {
    return alreadyHandlingException_;
  }

  void EventProcessor::terminateMachine() {
    if (machine_.get() != 0) {
      if (!machine_->terminated()) {
        machine_->process_event(statemachine::Stop());
      }
      else {
        FDEBUG(1) << "EventProcess::terminateMachine  The state machine was already terminated \n";
      }
      if (machine_->terminated()) {
        FDEBUG(1) << "The state machine reports it has been terminated (3)\n";
      }
      machine_.reset();
    }
  }
}
