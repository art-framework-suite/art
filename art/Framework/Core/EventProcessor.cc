#include "art/Framework/Core/EventProcessor.h"

#include "art/Framework/Core/Breakpoints.h"
#include "art/Framework/Core/EPStates.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceFactory.h"
#include "art/Framework/Principal/OccurrenceTraits.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Core/Schedule.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/System/CurrentModule.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Framework/Services/System/FloatingPointControl.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/GetPassID.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "art/Version/GetReleaseVersion.h"
#include "boost/thread/xtime.hpp"
#include "cetlib/exception_collector.h"
#include "cpp0x/functional"
#include "cpp0x/utility"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "tbb/task_scheduler_init.h"

#include <exception>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using fhicl::ParameterSet;

namespace {
  class SignalSentry {
  public:
    SignalSentry(SignalSentry const &) = delete;
    SignalSentry & operator=(SignalSentry const &) = delete;
    typedef sigc::signal<void> Sig;
    SignalSentry(Sig & pre, Sig & post)
      :
      post_(post) {
      pre();
    }
    ~SignalSentry() {
      post_();
    }
  private:
    Sig & post_;
  };

  void setupAsDefaultEmptySource(ParameterSet & p)
  {
    p.put("module_type", "EmptyEvent");
    p.put("module_label", "source");
    p.put("maxEvents", 1);
  }

  std::shared_ptr<art::InputSource>
  makeInput(ParameterSet const & params,
            std::string const & processName,
            art::MasterProductRegistry & preg,
            std::shared_ptr<art::ActivityRegistry> areg)
  {
    ParameterSet defaultEmptySource;
    setupAsDefaultEmptySource(defaultEmptySource);
    // find single source
    bool sourceSpecified = false;
    ParameterSet main_input = defaultEmptySource;
    try {
      if (!params.get_if_present("source", main_input)) {
        mf::LogInfo("EventProcessorSourceConfig")
            << "Could not find a source configuration: using default.";
      }
      // Fill in "ModuleDescription", in case the input source produces
      // any EDproducts,which would be registered in the
      // MasterProductRegistry.  Also fill in the process history item
      // for this process.
      art::ModuleDescription md;
      md.parameterSetID_ = main_input.id();
      md.moduleName_ = main_input.get<std::string>("module_type");
      md.moduleLabel_ = main_input.get<std::string>("module_label");
      md.processConfiguration_ =
        art::ProcessConfiguration(processName,
                                  params.id(),
                                  art::getReleaseVersion(),
                                  art::getPassID());
      sourceSpecified = true;
      art::InputSourceDescription isd(md, preg, *areg);
      return std::shared_ptr<art::InputSource>
             (art::InputSourceFactory::make(main_input, isd).release());
    }
    catch (art::Exception const & x) {
      if (sourceSpecified == false &&
          art::errors::Configuration == x.categoryCode()) {
        throw art::Exception(art::errors::Configuration, "FailedInputSource")
            << "Configuration of main input source has failed\n"
            << x;
      }
      else {
        throw;
      }
    }
    catch (cet::exception const & x) {
      throw art::Exception(art::errors::Configuration, "FailedInputSource")
          << "Configuration of main input source has failed\n"
          << x;
    }
    return std::shared_ptr<art::InputSource>();
  }

  //////////////////////////////////////////////////////////////////////
  // Functions to help prepare the services for initialization.

  typedef std::vector<ParameterSet> ParameterSets;

  void
  addService(std::string const & name, ParameterSets & service_set)
  {
    service_set.push_back(ParameterSet());
    service_set.back().put("service_type", name);
  }

  void
  addOptionalService(std::string const & name,
                     ParameterSet const & source,
                     ParameterSets & service_set)
  {
    ParameterSet pset;
    if (source.get_if_present(name, pset)) {
      pset.put("service_type", name);
      service_set.push_back(pset);
    }
  }

  void
  addService(std::string const & name,
             ParameterSet const & source,
             ParameterSets & service_set)
  {
    service_set.push_back(source.get<ParameterSet>(name, ParameterSet()));
    service_set.back().put("service_type", name);
  }

  void extractServices(ParameterSet const & services, ParameterSets & service_set)
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
    addOptionalService("RandomNumberGenerator", services, service_set);
    addOptionalService("SimpleMemoryCheck", services, service_set);
    addOptionalService("Timing", services, service_set);
    addOptionalService("TFileService", services, service_set);
    addService("FileCatalogMetadata", services, service_set);
    ParameterSet user_services = services.get<ParameterSet>("user", ParameterSet());
    std::vector<std::string> keys = user_services.get_pset_keys();
    for (std::vector<std::string>::iterator i = keys.begin(), e = keys.end(); i != e; ++i)
    { addService(*i, user_services, service_set); }
  }
}

// ----------- event processor functions ------------------

art::EventProcessor::EventProcessor(ParameterSet const & pset)
  :
  actReg_(new ActivityRegistry),
  mfStatusUpdater_(*actReg_),
  wreg_(actReg_),
  preg_(),
  serviceToken_(),
  input_(),
  schedule_(),
  act_table_(),
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
  ParameterSet const services = pset.get<ParameterSet>("services", ParameterSet());
  ParameterSet const scheduler = services.get<ParameterSet>("scheduler", ParameterSet());
  fileMode_ = scheduler.get<std::string>("fileMode", "");
  handleEmptyRuns_ = scheduler.get<bool>("handleEmptyRuns", true);
  handleEmptySubRuns_ = scheduler.get<bool>("handleEmptySubRuns", true);
  std::string const processName = pset.get<std::string>("process_name");

  // Services
  configureServices_(pset);
  ServiceRegistry::Operate operate(serviceToken_); // Make usable.
  serviceToken_.forceCreation();
  // FileCatalogMetadata needs to know about the process name.
  ServiceHandle<art::FileCatalogMetadata>()->addMetadata("process_name", processName);

  act_table_ = ActionTable(scheduler);
  input_ = makeInput(pset, processName, preg_, actReg_);
  // Old input sources may need this for now.
  input_->storeMPRforBrokenRandomAccess(preg_);

  initSchedules_(pset);
  FDEBUG(2) << pset.to_string() << std::endl;
  BranchIDListHelper::updateRegistries(preg_);
}

art::EventProcessor::~EventProcessor()
{
  // Make the services available while everything is being deleted.
  ServiceToken token = getToken_();
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
  terminateMachine_();
  // manually destroy all these thing that may need the services around
  schedule_.reset();
  input_.reset();
  wreg_.clear();
  actReg_.reset();
}

void
art::EventProcessor::beginJob()
{
  breakpoints::beginJob();
  // make the services available
  ServiceRegistry::Operate operate(serviceToken_);
  // NOTE:  This implementation assumes 'Job' means one call
  // the EventProcessor::run
  // If it really means once per 'application' then this code will
  // have to be changed.
  // Also have to deal with case where have 'run' then new Module
  // added and do 'run'
  // again.  In that case the newly added Module needs its 'beginJob'
  // to be called.
  try {
    input_->doBeginJob();
  }
  catch (cet::exception & e) {
    mf::LogError("BeginJob") << "A cet::exception happened while processing"
                             " the beginJob of the 'source'\n";
    e << "A cet::exception happened while processing"
      " the beginJob of the 'source'\n";
    throw;
  }
  catch (std::exception & e) {
    mf::LogError("BeginJob") << "A std::exception happened while processing"
                             " the beginJob of the 'source'\n";
    throw;
  }
  catch (...) {
    mf::LogError("BeginJob") << "An unknown exception happened while"
                             " processing the beginJob of the 'source'\n";
    throw;
  }
  schedule_->beginJob();
  actReg_->sPostBeginJob.invoke();
  Schedule::Workers aw_vec;
  schedule_->getAllWorkers(aw_vec);
  actReg_->sPostBeginJobWorkers.invoke(input_.get(), aw_vec);
}

void
art::EventProcessor::endJob()
{
  // Collects exceptions, so we don't throw before all operations are performed.
  cet::exception_collector c;
  // Make the services available
  ServiceRegistry::Operate operate(serviceToken_);
  c.call(std::bind(&EventProcessor::terminateMachine_, this));
  c.call(std::bind(&Schedule::endJob, schedule_.get()));
  c.call(std::bind(&InputSource::doEndJob, input_));
  c.call(std::bind(&decltype(ActivityRegistry::sPostEndJob)::invoke, actReg_->sPostEndJob));
}

void
art::EventProcessor::
configureServices_(ParameterSet const & pset)
{
  ParameterSet const services = pset.get<ParameterSet>("services", ParameterSet());
  ParameterSet const scheduler = services.get<ParameterSet>("scheduler", ParameterSet());
  bool const wantTracer = scheduler.get<bool>("wantTracer", false);
  // build a list of service parameter sets that will be used by the service registry
  ParameterSets service_set;
  extractServices(services, service_set);
  // configured based on optional parameters
  if (wantTracer) { addService("Tracer", service_set); }
  serviceToken_ = ServiceRegistry::createSet(service_set, *actReg_);
  // NOTE: the order here might be backwards, due to the "push_front" registering
  // that sigc++ does way in the guts of the add operation.
  // no configuration available
  serviceToken_.add(std::unique_ptr<CurrentModule>(new CurrentModule(*actReg_)));
  // special construction
  serviceToken_.add(std::unique_ptr<TriggerNamesService>(new TriggerNamesService(pset)));
  ParameterSet const fpc_pset = services.get<ParameterSet>("floating_point_control", ParameterSet());
  serviceToken_.add(std::unique_ptr<FloatingPointControl>(new FloatingPointControl(fpc_pset, *actReg_)));
}

void
art::EventProcessor::
initSchedules_(ParameterSet const & pset)
{
  ParameterSet const services =
    pset.get<ParameterSet>("services", ParameterSet());
  ParameterSet const scheduler =
    services.get<ParameterSet>("scheduler", ParameterSet());

  // Initialize TBB with desired number of threads.
  int num_threads =
    scheduler.get<int>("num_threads",
                       tbb::task_scheduler_init::default_num_threads());
  if (num_threads < 1) {
    throw Exception(errors::Configuration)
      << "Specified threads must be >= 1!\n";
  }
  tbb::task_scheduler_init init(num_threads);

  schedule_ =
    std::unique_ptr<Schedule>
    (new Schedule(pset,
                  ServiceRegistry::instance().get<TriggerNamesService>(),
                  wreg_,
                  preg_,
                  act_table_,
                  actReg_));
}

art::ServiceToken
art::EventProcessor::getToken_()
{
  return serviceToken_;
}

art::EventProcessor::StatusCode
art::EventProcessor::runToCompletion()
{
  StatusCode returnCode = runCommon_();
  if (machine_.get() != 0) {
    throw art::Exception(errors::LogicError)
        << "State machine not destroyed on exit from EventProcessor::runToCompletion\n"
        << "Please report this error to the Framework group\n";
  }
  return returnCode;
}

art::EventProcessor::StatusCode
art::EventProcessor::runCommon_()
{
  StatusCode returnCode = epSuccess;
  stateMachineWasInErrorState_ = false;
  // Make the services available
  ServiceRegistry::Operate operate(serviceToken_);
  if (machine_.get() == 0) {
    statemachine::FileMode fileMode;
    if (fileMode_.empty()) { fileMode = statemachine::FULLMERGE; }
    else if (fileMode_ == std::string("MERGE")) { fileMode = statemachine::MERGE; }
    else if (fileMode_ == std::string("NOMERGE")) { fileMode = statemachine::NOMERGE; }
    else if (fileMode_ == std::string("FULLMERGE")) { fileMode = statemachine::FULLMERGE; }
    else if (fileMode_ == std::string("FULLLUMIMERGE")) { fileMode = statemachine::FULLLUMIMERGE; }
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
      // Look for a shutdown signal
      {
        boost::mutex::scoped_lock sl(usr2_lock);
        if (art::shutdown_flag) {
          //changeState(mShutdownSignal);
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
      }
      // This should be impossible
      else {
        throw art::Exception(errors::LogicError)
            << "Unknown next item type passed to EventProcessor\n"
            << "Please report this error to the art developers\n";
      }
      if (machine_->terminated()) {
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
  catch (cet::exception & e) {
    terminateAbnormally_();
    e << "cet::exception caught in EventProcessor and rethrown\n";
    e << exceptionMessageSubRuns_;
    e << exceptionMessageRuns_;
    e << exceptionMessageFiles_;
    throw e;
  }
  catch (std::bad_alloc & e) {
    terminateAbnormally_();
    throw cet::exception("std::bad_alloc")
        << "The EventProcessor caught a std::bad_alloc exception and converted it to a cet::exception\n"
        << "The job has probably exhausted the virtual memory available to the process.\n"
        << exceptionMessageSubRuns_
        << exceptionMessageRuns_
        << exceptionMessageFiles_;
  }
  catch (std::exception & e) {
    terminateAbnormally_();
    throw cet::exception("StdException")
        << "The EventProcessor caught a std::exception and converted it to a cet::exception\n"
        << "Previous information:\n" << e.what() << "\n"
        << exceptionMessageSubRuns_
        << exceptionMessageRuns_
        << exceptionMessageFiles_;
  }
  catch (std::string const & e) {
    terminateAbnormally_();
    throw cet::exception("Unknown")
        << "The EventProcessor caught a string-based exception type and converted it to a cet::exception\n"
        << e
        << "\n"
        << exceptionMessageSubRuns_
        << exceptionMessageRuns_
        << exceptionMessageFiles_;
  }
  catch (char const * e) {
    terminateAbnormally_();
    throw cet::exception("Unknown")
        << "The EventProcessor caught a string-based exception type and converted it to a cet::exception\n"
        << e
        << "\n"
        << exceptionMessageSubRuns_
        << exceptionMessageRuns_
        << exceptionMessageFiles_;
  }
  catch (...) {
    terminateAbnormally_();
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
  if (stateMachineWasInErrorState_) {
    throw cet::exception("BadState")
        << "The boost state machine in the EventProcessor exited after\n"
        << "entering the Error state.\n";
  }
  return returnCode;
}

void
art::EventProcessor::readFile()
{
  actReg_->sPreOpenFile.invoke();
  FDEBUG(1) << " \treadFile\n";
  fb_ = input_->readFile(preg_);
  if (!fb_) {
    throw Exception(errors::LogicError)
        << "Source readFile() did not return a valid FileBlock: FileBlock "
        << "should be valid or readFile() should throw.\n";
  }
  actReg_->sPostOpenFile.invoke(fb_->fileName());
}

void
art::EventProcessor::closeInputFile()
{
  SignalSentry fileCloseSentry(actReg_->sPreCloseFile.signal_,
                               actReg_->sPostCloseFile.signal_);
  input_->closeFile();
  FDEBUG(1) << "\tcloseInputFile\n";
}

void
art::EventProcessor::openOutputFiles()
{
  schedule_->openOutputFiles(*fb_);
  FDEBUG(1) << "\topenOutputFiles\n";
}

void
art::EventProcessor::closeOutputFiles()
{
  schedule_->closeOutputFiles();
  FDEBUG(1) << "\tcloseOutputFiles\n";
}

void
art::EventProcessor::respondToOpenInputFile()
{
  schedule_->respondToOpenInputFile(*fb_);
  FDEBUG(1) << "\trespondToOpenInputFile\n";
}

void
art::EventProcessor::respondToCloseInputFile()
{
  schedule_->respondToCloseInputFile(*fb_);
  FDEBUG(1) << "\trespondToCloseInputFile\n";
}

void
art::EventProcessor::respondToOpenOutputFiles()
{
  schedule_->respondToOpenOutputFiles(*fb_);
  FDEBUG(1) << "\trespondToOpenOutputFiles\n";
}

void
art::EventProcessor::respondToCloseOutputFiles()
{
  schedule_->respondToCloseOutputFiles(*fb_);
  FDEBUG(1) << "\trespondToCloseOutputFiles\n";
}

void
art::EventProcessor::startingNewLoop()
{
  shouldWeStop_ = false;
  FDEBUG(1) << "\tstartingNewLoop\n";
}

bool
art::EventProcessor::endOfLoop()
{
  FDEBUG(1) << "\tendOfLoop\n";
  return true;
}

void
art::EventProcessor::rewindInput()
{
  input_->rewind();
  FDEBUG(1) << "\trewind\n";
}

void
art::EventProcessor::prepareForNextLoop()
{
  FDEBUG(1) << "\tprepareForNextLoop\n";
}

void
art::EventProcessor::writeSubRunCache()
{
  while (!principalCache_.noMoreSubRuns()) {
    schedule_->writeSubRun(principalCache_.lowestSubRun());
    principalCache_.deleteLowestSubRun();
  }
  FDEBUG(1) << "\twriteSubRunCache\n";
}

void
art::EventProcessor::writeRunCache()
{
  while (!principalCache_.noMoreRuns()) {
    schedule_->writeRun(principalCache_.lowestRun());
    principalCache_.deleteLowestRun();
  }
  FDEBUG(1) << "\twriteRunCache\n";
}

bool
art::EventProcessor::shouldWeCloseOutput() const
{
  FDEBUG(1) << "\tshouldWeCloseOutput\n";
  return schedule_->shouldWeCloseOutput();
}

void
art::EventProcessor::doErrorStuff()
{
  FDEBUG(1) << "\tdoErrorStuff\n";
  mf::LogError("StateMachine")
      << "The EventProcessor state machine encountered an unexpected event\n"
      "and went to the error state\n"
      "Will attempt to terminate processing normally\n"
      "This likely indicates a bug in an input module, corrupted input, or both\n";
  stateMachineWasInErrorState_ = true;
}

void
art::EventProcessor::beginRun(int run)
{
  RunPrincipal & runPrincipal = principalCache_.runPrincipal(run);
  schedule_->processOneOccurrence<OccurrenceTraits<RunPrincipal, BranchActionBegin> >(runPrincipal);
  FDEBUG(1) << "\tbeginRun " << run << "\n";
}

void
art::EventProcessor::endRun(int run)
{
  RunPrincipal & runPrincipal = principalCache_.runPrincipal(run);
  //input_->doEndRun(runPrincipal);
  schedule_->processOneOccurrence<OccurrenceTraits<RunPrincipal, BranchActionEnd> >(runPrincipal);
  FDEBUG(1) << "\tendRun " << run << "\n";
}

void
art::EventProcessor::beginSubRun(int run, int subRun)
{
  SubRunPrincipal & subRunPrincipal = principalCache_.subRunPrincipal(run, subRun);
  // NOTE: Using 0 as the event number for the begin of a subRun block is a bad idea
  // subRun blocks know their start and end times why not also start and end events?
  schedule_->processOneOccurrence<OccurrenceTraits<SubRunPrincipal, BranchActionBegin> >(subRunPrincipal);
  FDEBUG(1) << "\tbeginSubRun " << run << "/" << subRun << "\n";
}

void
art::EventProcessor::endSubRun(int run, int subRun)
{
  SubRunPrincipal & subRunPrincipal = principalCache_.subRunPrincipal(run, subRun);
  //NOTE: Using the max event number for the end of a subRun block is a bad idea
  // subRun blocks know their start and end times why not also start and end events?
  schedule_->processOneOccurrence<OccurrenceTraits<SubRunPrincipal, BranchActionEnd> >(subRunPrincipal);
  FDEBUG(1) << "\tendSubRun " << run << "/" << subRun << "\n";
}

int
art::EventProcessor::readAndCacheRun()
{
  SignalSentry runSourceSentry(actReg_->sPreSourceRun.signal_,
                               actReg_->sPostSourceRun.signal_);
  principalCache_.insert(input_->readRun());
  FDEBUG(1) << "\treadAndCacheRun " << "\n";
  return principalCache_.runPrincipal().run();
}

int
art::EventProcessor::readAndCacheSubRun()
{
  SignalSentry subRunSourceSentry(actReg_->sPreSourceSubRun.signal_,
                                  actReg_->sPostSourceSubRun.signal_);
  principalCache_.insert(input_->readSubRun(principalCache_.runPrincipalPtr()));
  FDEBUG(1) << "\treadAndCacheSubRun " << "\n";
  return principalCache_.subRunPrincipal().subRun();
}

void
art::EventProcessor::writeRun(int run)
{
  schedule_->writeRun(principalCache_.runPrincipal(run));
  FDEBUG(1) << "\twriteRun " << run << "\n";
}

void
art::EventProcessor::deleteRunFromCache(int run)
{
  principalCache_.deleteRun(run);
  FDEBUG(1) << "\tdeleteRunFromCache " << run << "\n";
}

void
art::EventProcessor::writeSubRun(int run, int subRun)
{
  schedule_->writeSubRun(principalCache_.subRunPrincipal(run, subRun));
  FDEBUG(1) << "\twriteSubRun " << run << "/" << subRun << "\n";
}

void
art::EventProcessor::deleteSubRunFromCache(int run, int subRun)
{
  principalCache_.deleteSubRun(run, subRun);
  FDEBUG(1) << "\tdeleteSubRunFromCache " << run << "/" << subRun << "\n";
}

void
art::EventProcessor::readEvent()
{
  sm_evp_ = input_->readEvent(principalCache_.subRunPrincipalPtr());
  FDEBUG(1) << "\treadEvent\n";
}

void
art::EventProcessor::processEvent()
{
  schedule_->processOneOccurrence<OccurrenceTraits<EventPrincipal, BranchActionBegin> >(*sm_evp_);
  FDEBUG(1) << "\tprocessEvent\n";
}

bool
art::EventProcessor::shouldWeStop() const
{
  FDEBUG(1) << "\tshouldWeStop\n";
  if (shouldWeStop_) { return true; }
  return schedule_->terminate();
}

void
art::EventProcessor::setExceptionMessageFiles(std::string & message)
{
  exceptionMessageFiles_ = message;
}

void
art::EventProcessor::setExceptionMessageRuns(std::string & message)
{
  exceptionMessageRuns_ = message;
}

void
art::EventProcessor::setExceptionMessageSubRuns(std::string & message)
{
  exceptionMessageSubRuns_ = message;
}

bool
art::EventProcessor::alreadyHandlingException() const
{
  return alreadyHandlingException_;
}

void
art::EventProcessor::terminateMachine_()
{
  if (machine_.get() != 0) {
    if (!machine_->terminated()) {
      machine_->process_event(statemachine::Stop());
    }
    else {
      FDEBUG(1) << "EventProcess::terminateMachine_: The state machine was already terminated \n";
    }
    if (machine_->terminated()) {
      FDEBUG(1) << "The state machine reports it has been terminated (3)\n";
    }
    machine_.reset();
  }
}

void
art::EventProcessor::terminateAbnormally_() try
{
  alreadyHandlingException_ = true;
  if (ServiceRegistry::instance().isAvailable<RandomNumberGenerator>()) {
    ServiceHandle<RandomNumberGenerator>()->saveToFile_();
  }
  terminateMachine_();
  alreadyHandlingException_ = false;
}
catch (...)
{
}
