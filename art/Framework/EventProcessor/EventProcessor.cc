#include "art/Framework/EventProcessor/EventProcessor.h"

#include "art/Framework/Core/Breakpoints.h"
#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceFactory.h"
#include "art/Framework/EventProcessor/detail/writeSummary.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/PrincipalPackages.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/System/CurrentModule.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Framework/Services/System/FloatingPointControl.h"
#include "art/Framework/Services/System/PathSelection.h"
#include "art/Framework/Services/System/ScheduleContext.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/bold_fontify.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/GetPassID.h"
#include "cetlib/exception_collector.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/detail/validationException.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cassert>
#include <exception>
#include <iomanip>
#include <string>
#include <utility>
#include <vector>

using fhicl::ParameterSet;

namespace {

  // Most signals.
  class SignalSentry {
  public:
    using PreSig_t  = art::GlobalSignal<art::detail::SignalResponseType::FIFO, void>;
    using PostSig_t = art::GlobalSignal<art::detail::SignalResponseType::LIFO, void>;

    SignalSentry(SignalSentry const&) = delete;
    SignalSentry& operator=(SignalSentry const&) = delete;

    explicit SignalSentry(PreSig_t& pre, PostSig_t& post) : post_{post}
    {
      pre.invoke();
    }
    ~SignalSentry()
    {
      post_.invoke();
    }
  private:
    PostSig_t& post_;
  };

  ////////////////////////////////////
  void setupAsDefaultEmptySource(ParameterSet& p)
  {
    p.put("module_type", "EmptyEvent");
    p.put("module_label", "source");
    p.put("maxEvents", 1);
  }

  std::unique_ptr<art::InputSource>
  makeInput(ParameterSet const& params,
            std::string const& processName,
            art::MasterProductRegistry& preg,
            art::ActivityRegistry& areg)
  {
    ParameterSet defaultEmptySource;
    setupAsDefaultEmptySource(defaultEmptySource);
    // find single source
    bool sourceSpecified {false};
    ParameterSet main_input {defaultEmptySource};
    try {
      if (!params.get_if_present("source", main_input)) {
        mf::LogInfo("EventProcessorSourceConfig")
          << "Could not find a source configuration: using default.";
      }
      // Fill in "ModuleDescription", in case the input source produces
      // any EDproducts,which would be registered in the
      // MasterProductRegistry.  Also fill in the process history item
      // for this process.
      art::ModuleDescription const md {main_input.id(),
                                       main_input.get<std::string>("module_type"),
                                       main_input.get<std::string>("module_label"),
                                       art::ProcessConfiguration{processName,
                                                                 params.id(),
                                                                 art::getReleaseVersion(),
                                                                 art::getPassID()}};
      sourceSpecified = true;
      art::InputSourceDescription isd{md, preg, areg};
      try {
        return std::unique_ptr<art::InputSource>(art::InputSourceFactory::make(main_input, isd));
      }
      catch(fhicl::detail::validationException const& e){
        throw art::Exception(art::errors::Configuration)
          << "\n\nModule label: " << art::detail::bold_fontify(md.moduleLabel())
          <<   "\nmodule_type : " << art::detail::bold_fontify(md.moduleName())
          << "\n\n" << e.what();
      }
    }
    catch (art::Exception const& x) {
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
    catch (cet::exception const& x) {
      throw art::Exception(art::errors::Configuration, "FailedInputSource")
        << "Configuration of main input source has failed\n"
        << x;
    }
    return std::unique_ptr<art::InputSource>();
  }

  std::string spaces(unsigned const n) { return std::string(n,' '); }

}

art::EventProcessor::EventProcessor(ParameterSet const& pset)
  :
  act_table_{pset.get<ParameterSet>("services.scheduler")},
  actReg_(),
  mfStatusUpdater_{actReg_},
  pathManager_{pset, preg_, act_table_, actReg_},
  serviceDirector_{initServices_(pset, actReg_, serviceToken_)},
  handleEmptyRuns_{pset.get<bool>("services.scheduler.handleEmptyRuns", true)},
  handleEmptySubRuns_{pset.get<bool>("services.scheduler.handleEmptySubRuns", true)}
{
  servicesActivate_(serviceToken_);
  serviceToken_.forceCreation();

  std::string const processName {pset.get<std::string>("process_name")};

  // Services
  // System service FileCatalogMetadata needs to know about the process name.
  ServiceHandle<art::FileCatalogMetadata>()->addMetadataString("process_name", processName);

  input_ = makeInput(pset, processName, preg_, actReg_);
  endPathExecutor_ = std::make_unique<EndPathExecutor>(pathManager_,
                                                       act_table_,
                                                       actReg_,
                                                       preg_);
  initSchedules_(pset);
  FDEBUG(2) << pset.to_string() << std::endl;
  BranchIDListHelper::updateRegistries(preg_);
  servicesDeactivate_();
}

art::EventProcessor::~EventProcessor()
{
  // Services must stay usable until they go out of scope, meaning
  // that modules may (say) use services in their destructors.
  servicesActivate_(serviceToken_);
  // The state machine should have already been cleaned up and
  // destroyed at this point by a call to EndJob or earlier when it
  // completed processing events, but if it has not been we'll take
  // care of it here at the last moment.  This could cause problems if
  // we are already handling an exception and another one is thrown
  // here ..  For a critical executable the solution to this problem
  // is for the code using the EventProcessor to explicitly call
  // EndJob or use runToCompletion, then the next line of code is
  // never executed.
  terminateMachine_();
}

void
art::EventProcessor::beginJob()
{
  FDEBUG(1) << spaces(8) << "beginJob\n";
  breakpoints::beginJob();
  // make the services available
  ServiceRegistry::Operate op {serviceToken_};
  // NOTE: This implementation assumes 'Job' means one call the
  // EventProcessor::run. If it really means once per 'application'
  // then this code will have to be changed.  Also have to deal with
  // case where have 'run' then new Module added and do 'run' again.
  // In that case the newly added Module needs its 'beginJob' to be
  // called.
  try {
    input_->doBeginJob();
  }
  catch (cet::exception& e) {
    mf::LogError("BeginJob") << "A cet::exception happened while processing"
                             " the beginJob of the 'source'\n";
    e << "A cet::exception happened while processing"
      " the beginJob of the 'source'\n";
    throw;
  }
  catch (std::exception const& e) {
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
  endPathExecutor_->beginJob();
  actReg_.sPostBeginJob.invoke();

  invokePostBeginJobWorkers_();
}

void
art::EventProcessor::endJob()
{
  FDEBUG(1) << spaces(8) << "endJob\n";
  // Collects exceptions, so we don't throw before all operations are performed.
  cet::exception_collector c;
  // Make the services available
  ServiceRegistry::Operate op {serviceToken_};
  //  c.call([this](){ terminateMachine_(); });
  c.call([this](){ schedule_->endJob(); });
  c.call([this](){ endPathExecutor_->endJob(); });
  bool summarize = ServiceHandle<TriggerNamesService>()->wantSummary();
  c.call([this,summarize](){ detail::writeSummary(pathManager_, summarize); });
  c.call([this](){ input_->doEndJob(); });
  c.call([this](){ actReg_.sPostEndJob.invoke(); });
}

art::ServiceDirector
art::EventProcessor::initServices_(ParameterSet const& top_pset,
                                   ActivityRegistry& areg,
                                   ServiceToken& token)
{
  auto services = top_pset.get<ParameterSet>("services", {});

  // Check if disallowed 'user' table is specified:


  // Save and non-standard service configs, "floating_point_control" to
  // prevent ServiceDirector trying to make one itself.
  auto const fpc_pset = services.get<ParameterSet>("floating_point_control", {});
  services.erase("floating_point_control");

  // Remove non-standard non-service config, "message."
  services.erase("message");

  // Deal with possible configuration for system service requiring
  // special construction:
  auto const pathSelection = services.get<ParameterSet>("PathSelection", {});
  services.erase("PathSelection");

  // Create the service director and all user-configured services.
  ServiceDirector director{std::move(services), areg, token};

  // Services requiring special construction.
  director.addSystemService<CurrentModule>(areg);
  director.addSystemService<TriggerNamesService>(top_pset, pathManager_.triggerPathNames());
  director.addSystemService<FloatingPointControl>(fpc_pset, areg);
  director.addSystemService<ScheduleContext>();
  if (!pathSelection.is_empty()) {
    director.addSystemService<PathSelection>(*this);
  }
  return std::move(director);
}

void
art::EventProcessor::initSchedules_(ParameterSet const& pset)
{
  // Initialize TBB with desired number of threads.
  int const num_threads = pset.get<int>("services.num_threads",
                                        tbb::task_scheduler_init::default_num_threads());
  tbbManager_.initialize(num_threads);

  schedule_ = std::make_unique<Schedule>(ScheduleID::first(),
                                         pathManager_,
                                         pset,
                                         ServiceRegistry::instance().get<TriggerNamesService>(),
                                         preg_,
                                         act_table_,
                                         actReg_);
}

void
art::EventProcessor::invokePostBeginJobWorkers_()
{
  // Need to convert multiple lists of workers into a long list that the
  // postBeginJobWorkers callbacks can understand.
  std::vector<Worker *> allWorkers;
  allWorkers.reserve(pathManager_.triggerPathsInfo(ScheduleID::first()).workers().size() +
                     pathManager_.endPathInfo().workers().size());
  auto workerStripper = [&allWorkers](WorkerMap::value_type const& val) {
    allWorkers.emplace_back(val.second.get());
  };
  cet::for_all(pathManager_.triggerPathsInfo(ScheduleID::first()).workers(),
               workerStripper);
  cet::for_all(pathManager_.endPathInfo().workers(),
                workerStripper);
  actReg_.sPostBeginJobWorkers.invoke(input_.get(), allWorkers);
}

art::ServiceToken
art::EventProcessor::getToken_()
{
  return serviceToken_;
}

art::EventProcessor::StatusCode
art::EventProcessor::runToCompletion()
{
  StatusCode returnCode {epSuccess};
  stateMachineWasInErrorState_ = false;
  // Make the services available
  ServiceRegistry::Operate op {serviceToken_};
  machine_ = std::make_unique<statemachine::Machine>(this);
  // machine_->initiate();

  try {
    process<Level::Job>();

  // try {
  //   while (true) {
  //     auto const nextLevel_ = advance();
  //     // Look for a shutdown signal
  //     {
  //       boost::mutex::scoped_lock sl {usr2_lock};
  //       if (art::shutdown_flag > 0) {
  //         returnCode = epSignal;
  //         machine_->process_event(statemachine::Stop());
  //         break;
  //       }
  //     }
  //     if (nextLevel_ == Level::Done) {
  //       machine_->process_event(statemachine::Stop());
  //     }
  //     else if (nextLevel_ == Level::InputFile) {
  //       machine_->process_event(statemachine::InputFile());
  //     }
  //     else if (nextLevel_ == Level::Run) {
  //       machine_->process_event(statemachine::Run(input_->run()));
  //     }
  //     else if (nextLevel_ == Level::SubRun) {
  //       machine_->process_event(statemachine::SubRun(input_->subRun()));
  //     }
  //     else if (nextLevel_ == Level::Event) {
  //       machine_->process_event(statemachine::Event());
  //     }
  //     else {
  //       throw art::Exception(errors::LogicError)
  //           << "Unknown next item type passed to EventProcessor\n"
  //           << "Please report this error to the art developers\n";
  //     }
  //     if (machine_->terminated()) {
  //       break;
  //     }
  //   }  // End of loop over state machine events
  } // Try block

  // Some comments on exception handling related to the boost state
  // machine:
  //
  // Some states used in the machine are special because they perform
  // actions while the machine is being terminated, actions such as
  // close files, call endRun, call endSubRun etc ..  Each of these
  // states has two functions that perform these actions.  The
  // functions are almost identical.  The major difference is that one
  // version catches all exceptions and the other lets exceptions pass
  // through.  The destructor catches them and the other function
  // named "exit" lets them pass through.  On a normal termination,
  // boost will always call "exit" and then the state destructor.  In
  // our state classes, the the destructors do nothing if the exit
  // function already took care of things.  Here's the interesting
  // part.  When boost is handling an exception the "exit" function is
  // not called (a boost feature).
  //
  // If an exception occurs while the boost machine is in control
  // (which usually means inside a process_event call), then the boost
  // state machine destroys its states and "terminates" itself.  This
  // is already done before we hit the catch blocks below. In this
  // case the call to terminateMachine below only destroys an already
  // terminated state machine.  Because exit is not called, the state
  // destructors handle cleaning up subRuns, runs, and files.  The
  // destructors swallow all exceptions and only pass through the
  // exceptions messages which are tacked onto the original exception
  // below.
  //
  // If an exception occurs when the boost state machine is not in
  // control (outside the process_event functions), then boost cannot
  // destroy its own states.  The terminateMachine function below
  // takes care of that.  The flag "alreadyHandlingException" is set
  // true so that the state exit functions do nothing (and cannot
  // throw more exceptions while handling the first).  Then the state
  // destructors take care of this because exit did nothing.
  //
  // One tricky aspect of the state machine is that things which can
  // throw should not be invoked by the state machine while another
  // exception is being handled.
  //
  // Another tricky aspect is that it appears to be important to
  // terminate the state machine before invoking its destructor.
  // We've seen crashes which are not understood when that is not
  // done.  Maintainers of this code should be careful about this.
  catch (art::Exception& e) {
    if (e.categoryCode() == art::errors::SignalReceived) {
      returnCode = epSignal;
    }
    else {
      terminateAbnormally_();
      e << "art::Exception caught in EventProcessor and rethrown\n";
      e << exceptionMessageEvents_;
      e << exceptionMessageSubRuns_;
      e << exceptionMessageRuns_;
      e << exceptionMessageFiles_;
      throw e;
    }
  }
  catch (cet::exception& e) {
    terminateAbnormally_();
    e << "cet::exception caught in EventProcessor and rethrown\n";
    e << exceptionMessageEvents_;
    e << exceptionMessageSubRuns_;
    e << exceptionMessageRuns_;
    e << exceptionMessageFiles_;
    throw e;
  }
  catch (std::bad_alloc const& e) {
    terminateAbnormally_();
    throw cet::exception("std::bad_alloc")
        << "The EventProcessor caught a std::bad_alloc exception and converted it to a cet::exception\n"
        << "The job has probably exhausted the virtual memory available to the process.\n"
        << exceptionMessageEvents_
        << exceptionMessageSubRuns_
        << exceptionMessageRuns_
        << exceptionMessageFiles_;
  }
  catch (std::exception const& e) {
    terminateAbnormally_();
    throw cet::exception("StdException")
        << "The EventProcessor caught a std::exception and converted it to a cet::exception\n"
        << "Previous information:\n" << e.what() << "\n"
        << exceptionMessageEvents_
        << exceptionMessageSubRuns_
        << exceptionMessageRuns_
        << exceptionMessageFiles_;
  }
  catch (std::string const& e) {
    terminateAbnormally_();
    throw cet::exception("Unknown")
        << "The EventProcessor caught a string-based exception type and converted it to a cet::exception\n"
        << e
        << "\n"
        << exceptionMessageEvents_
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
        << exceptionMessageEvents_
        << exceptionMessageSubRuns_
        << exceptionMessageRuns_
        << exceptionMessageFiles_;
  }
  catch (...) {
    terminateAbnormally_();
    throw cet::exception("Unknown")
        << "The EventProcessor caught an unknown exception type and converted it to a cet::exception\n"
        << exceptionMessageEvents_
        << exceptionMessageSubRuns_
        << exceptionMessageRuns_
        << exceptionMessageFiles_;
  }
  if (machine_->terminated()) {
    FDEBUG(2) << "The state machine reports it has been terminated\n";
  }
  if (stateMachineWasInErrorState_) {
    throw cet::exception("BadState")
        << "The boost state machine in the EventProcessor exited after\n"
        << "entering the Error state.\n";
  }
  return returnCode;
}

void
art::EventProcessor::levelProcessed()
{
  assert(!activeLevels_.empty());
  activeLevels_.pop_back();
}

art::Level
art::EventProcessor::advance()
{
  auto const itemType = input_->nextItemType();
  FDEBUG(1) << spaces(4) << "*** nextItemType: " << itemType << " ***\n";
  switch(itemType) {
  case input::IsStop: return Level::Done;
  case input::IsFile: return Level::InputFile;
  case input::IsRun: return Level::Run;
  case input::IsSubRun: return Level::SubRun;
  case input::IsEvent: return Level::Event;
  case input::IsInvalid: {
    throw art::Exception{art::errors::LogicError} << "Invalid next item type presented to state machine.";
  }
  }
  return Level::Empty;
}

void
art::EventProcessor::maybeTriggerOutputFileSwitch()
{
  assert(stagingAllowed());

  if (!outputsToClose()) return;

  // Don't trigger another switch if one is already in progress!
  if (switchInProgress()) return;

  machine_->post_event(statemachine::Pause{});
  machine_->post_event(statemachine::SwitchOutputFiles{});
  setSwitchInProgress(true);
}

void
art::EventProcessor::openInputFile()
{
  actReg_.sPreOpenFile.invoke();
  FDEBUG(1) << spaces(8) << "openInputFile\n";
  fb_ = input_->readFile(preg_);
  if (!fb_) {
    throw Exception(errors::LogicError)
        << "Source readFile() did not return a valid FileBlock: FileBlock "
        << "should be valid or readFile() should throw.\n";
  }
  actReg_.sPostOpenFile.invoke(fb_->fileName());
  respondToOpenInputFile();
}

void
art::EventProcessor::closeAllFiles()
{
  closeAllOutputFiles();
  closeInputFile();
}

void
art::EventProcessor::closeInputFile()
{
  incrementInputFileNumber();
  recordOutputClosureRequests(Boundary::InputFile);
  if (outputsToClose()) {
    closeSomeOutputFiles();
  }
  respondToCloseInputFile();
  SignalSentry fileCloseSentry {actReg_.sPreCloseFile, actReg_.sPostCloseFile};
  input_->closeFile();
  FDEBUG(1) << spaces(8) << "closeInputFile\n";
}

void
art::EventProcessor::openAllOutputFiles()
{
  endPathExecutor_->openAllOutputFiles(*fb_);
  FDEBUG(1) << spaces(8) << "openAllOutputFiles\n";
}

void
art::EventProcessor::closeAllOutputFiles()
{
  if (!someOutputsOpen()) return;
  respondToCloseOutputFiles();

  endPathExecutor_->closeAllOutputFiles();
  FDEBUG(1) << spaces(8) << "closeAllOutputFiles\n";
}

void
art::EventProcessor::openSomeOutputFiles()
{
  if (!outputsToOpen()) return;

  endPathExecutor_->openSomeOutputFiles(*fb_);
  FDEBUG(1) << spaces(8) << "openSomeOutputFiles\n";
  respondToOpenOutputFiles();
  setSwitchInProgress(false);
  setStagingAllowed(true);
}

void
art::EventProcessor::setOutputFileStatus(OutputFileStatus const ofs)
{
  endPathExecutor_->setOutputFileStatus(ofs);
  FDEBUG(1) << spaces(8) << "setOutputFileStatus\n";
}

void
art::EventProcessor::closeSomeOutputFiles()
{
  // Precondition: there are SOME output files that have been
  //               flagged as needing to close.  Otherwise,
  //               'respondtoCloseOutputFiles' will be needlessly
  //               called.
  assert(outputsToClose());
  respondToCloseOutputFiles();
  endPathExecutor_->closeSomeOutputFiles();
  FDEBUG(1) << spaces(8) << "closeSomeOutputFiles\n";
}

void
art::EventProcessor::respondToOpenInputFile()
{
  schedule_->respondToOpenInputFile(*fb_);
  endPathExecutor_->respondToOpenInputFile(*fb_);
  FDEBUG(1) << spaces(8) << "respondToOpenInputFile\n";
}

void
art::EventProcessor::respondToCloseInputFile()
{
  schedule_->respondToCloseInputFile(*fb_);
  endPathExecutor_->respondToCloseInputFile(*fb_);
  FDEBUG(1) << spaces(8) << "respondToCloseInputFile\n";
}

void
art::EventProcessor::respondToOpenOutputFiles()
{
  schedule_->respondToOpenOutputFiles(*fb_);
  endPathExecutor_->respondToOpenOutputFiles(*fb_);
  FDEBUG(1) << spaces(8) << "respondToOpenOutputFiles\n";
}

void
art::EventProcessor::respondToCloseOutputFiles()
{
  schedule_->respondToCloseOutputFiles(*fb_);
  endPathExecutor_->respondToCloseOutputFiles(*fb_);
  FDEBUG(1) << spaces(8) << "respondToCloseOutputFiles\n";
}

void
art::EventProcessor::rewindInput()
{
  input_->rewind();
  FDEBUG(1) << spaces(8) << "rewind\n";
}

void
art::EventProcessor::doErrorStuff()
{
  FDEBUG(1) << spaces(8) << "doErrorStuff\n";
  mf::LogError("StateMachine")
      << "The EventProcessor state machine encountered an unexpected event\n"
      "and went to the error state\n"
      "Will attempt to terminate processing normally\n"
      "This likely indicates a bug in an input module, corrupted input, or both\n";
  stateMachineWasInErrorState_ = true;
}

//=============================================
// Run level

void
art::EventProcessor::setupCurrentRun()
{
  finalizeRunEnabled_ = true;
  runException_ = true;
  readRun();
  runException_ = false;
  if (handleEmptyRuns_) {
    beginRun();
    beginRunCalled_ = true;
  }
}

void
art::EventProcessor::readRun()
{
  SignalSentry runSourceSentry {actReg_.sPreSourceRun, actReg_.sPostSourceRun};
  runPrincipal_ = input_->readRun();
  endPathExecutor_->seedRunRangeSet(input_->runRangeSetHandler());
  FDEBUG(1) << spaces(8) << "readRun.....................(" << runPrincipalID() << ")\n";
}

void
art::EventProcessor::beginRun()
{
  if (runPrincipalID().isFlush()) return;

  finalizeRunEnabled_ = true;
  runException_ = true;
  process_<Begin<Level::Run>>(*runPrincipal_);
  FDEBUG(1) << spaces(8) << "beginRun....................(" << runPrincipal_->id() << ")\n";
  runException_ = false;
}

void
art::EventProcessor::beginRunIfNotDoneAlready()
{
  if (beginRunCalled_) return;

  beginRun();
  beginRunCalled_ = true;
}

void
art::EventProcessor::setRunAuxiliaryRangeSetID()
{
  endPathExecutor_->setAuxiliaryRangeSetID(*runPrincipal_);
  FDEBUG(1) << spaces(8) << "setRunAuxiliaryRangeSetID...(" << runPrincipalID() << ")\n";
}

void
art::EventProcessor::endRun()
{
  runException_ = true;
  // Precondition: The RunID does not correspond to a flush ID. --
  // N.B. The flush flag is not explicitly checked here since endRun
  // is only called from finalizeRun, which is where the check
  // happens.
  RunID const run {runPrincipal_->id()};
  assert(!run.isFlush());
  process_<End<Level::Run>>(*runPrincipal_);
  FDEBUG(1) << spaces(8) << "endRun......................(" << run << ")\n";
  runException_ = false;
}

void
art::EventProcessor::writeRun()
{
  // Precondition: The RunID does not correspond to a flush ID.
  RunID const r {runPrincipal_->id()};
  assert(!r.isFlush());
  endPathExecutor_->writeRun(*runPrincipal_);
  FDEBUG(1) << spaces(8) << "writeRun....................(" << r << ")\n";
}

void
art::EventProcessor::finalizeRun()
{
  if (!finalizeRunEnabled_) return;
  if (runException_) return;
  if (runPrincipalID().isFlush()) return;

  runException_ = true;
  openSomeOutputFiles();
  setRunAuxiliaryRangeSetID();
  if (beginRunCalled_)
    endRun();
  writeRun();

  // Staging is not allowed whenever 'maybeTriggerOutputFileSwitch'
  // is called due to exiting a 'Pause' state.
  if (stagingAllowed_) {
    recordOutputClosureRequests(Boundary::Run);
    //    maybeTriggerOutputFileSwitch();
  }

  beginRunCalled_ = false;
  finalizeRunEnabled_ = false;
  runException_ = false;
}

//=============================================
// SubRun level

void
art::EventProcessor::setupCurrentSubRun()
{
  finalizeSubRunEnabled_ = true;
  assert(runPrincipalID().isValid());
  subRunException_ = true;
  readSubRun();
  subRunException_ = false;
  if (handleEmptySubRuns_) {
    beginRunIfNotDoneAlready();
    beginSubRun();
    beginSubRunCalled_ = true;
  }
}

void
art::EventProcessor::readSubRun()
{
  SignalSentry subRunSourceSentry {actReg_.sPreSourceSubRun, actReg_.sPostSourceSubRun};
  subRunPrincipal_ = input_->readSubRun(runPrincipal_.get());
  endPathExecutor_->seedSubRunRangeSet(input_->subRunRangeSetHandler());
  FDEBUG(1) << spaces(8) << "readSubRun..................(" << subRunPrincipalID() << ")\n";
}

void
art::EventProcessor::beginSubRun()
{
  if (subRunPrincipalID().isFlush()) return;

  finalizeSubRunEnabled_ = true;
  subRunException_ = true;
  process_<Begin<Level::SubRun>>(*subRunPrincipal_);
  FDEBUG(1) << spaces(8) << "beginSubRun.................(" << subRunPrincipal_->id() <<")\n";
  subRunException_ = false;
}

void
art::EventProcessor::beginSubRunIfNotDoneAlready()
{
  if (beginSubRunCalled_) return;

  beginSubRun();
  beginSubRunCalled_ = true;
}

void
art::EventProcessor::setSubRunAuxiliaryRangeSetID()
{
  endPathExecutor_->setAuxiliaryRangeSetID(*subRunPrincipal_);
  FDEBUG(1) << spaces(8) << "setSubRunAuxiliaryRangeSetID(" << subRunPrincipalID() << ")\n";
}

void
art::EventProcessor::endSubRun()
{
  subRunException_ = true;
  // Precondition: The SubRunID does not correspond to a flush ID.
  // Note: the flush flag is not explicitly checked here since
  // endSubRun is only called from finalizeSubRun, which is where the
  // check happens.
  SubRunID const sr {subRunPrincipal_->id()};
  assert(!sr.isFlush());
  process_<End<Level::SubRun>>(*subRunPrincipal_);
  FDEBUG(1) << spaces(8) << "endSubRun...................(" << sr << ")\n";
  subRunException_ = false;
}

void
art::EventProcessor::writeSubRun()
{
  // Precondition: The SubRunID does not correspond to a flush ID.
  SubRunID const& sr {subRunPrincipal_->id()};
  assert(!sr.isFlush());
  endPathExecutor_->writeSubRun(*subRunPrincipal_);
  FDEBUG(1) << spaces(8) << "writeSubRun.................(" << sr << ")\n";
}

void
art::EventProcessor::finalizeSubRun()
{
  if (!finalizeSubRunEnabled_) return;
  if (subRunException_) return;
  if (subRunPrincipalID().isFlush()) return;

  subRunException_ = true;
  openSomeOutputFiles();
  setSubRunAuxiliaryRangeSetID();
  if (beginSubRunCalled_) {
    endSubRun();
  }
  writeSubRun();

  // Staging is not allowed whenever 'maybeTriggerOutputFileSwitch'
  // is called due to exiting a 'Pause' state.
  if (stagingAllowed_) {
    recordOutputClosureRequests(Boundary::SubRun);
    //    maybeTriggerOutputFileSwitch();
  }

  finalizeSubRunEnabled_ = false;
  beginSubRunCalled_ = false;
  subRunException_ = false;
}

art::RunID
art::EventProcessor::runPrincipalID() const
{
  return runPrincipal_ ? runPrincipal_->id() : art::RunID{};
}

art::SubRunID
art::EventProcessor::subRunPrincipalID() const
{
  return subRunPrincipal_ ? subRunPrincipal_->id() : art::SubRunID{};
}

art::EventID
art::EventProcessor::eventPrincipalID() const
{
  return eventPrincipal_ ? eventPrincipal_->id() : art::EventID{};
}

//=============================================
// Event level

void
art::EventProcessor::readEvent()
{
  finalizeEventEnabled_ = true;
  SignalSentry sourceSentry {actReg_.sPreSource, actReg_.sPostSource};
  eventPrincipal_ = input_->readEvent(subRunPrincipal_.get());
  FDEBUG(1) << spaces(8) << "readEvent...................(" << eventPrincipalID() << ")\n";
}

void
art::EventProcessor::processEvent()
{
  EventID const& id {eventPrincipal_->id()};
  // Precondition: The EventID does not correspond to a flush ID.
  assert(!id.isFlush());
  process_<Do<Level::Event>>(*eventPrincipal_);
  FDEBUG(1) << spaces(8) << "processEvent................(" << id << ")\n";
}

void
art::EventProcessor::writeEvent()
{
  EventID const& id {eventPrincipal_->id()};
  // Precondition: The EventID does not correspond to a flush ID.
  assert(!id.isFlush());
  endPathExecutor_->writeEvent(*eventPrincipal_);
  FDEBUG(1) << spaces(8) << "writeEvent..................(" << id << ")\n";
  eventPrincipal_.reset();
}

void
art::EventProcessor::finalizeEvent()
{
  if (!finalizeEventEnabled_) return;
  if (eventException_) return;
  if (eventPrincipalID().isFlush()) return;

  eventException_ = true;
  openSomeOutputFiles();
  writeEvent();

  // Staging is not allowed whenever 'maybeTriggerOutputFileSwitch'
  // is called due to exiting a 'Pause' state.
  if (stagingAllowed_) {
    recordOutputClosureRequests(Boundary::Event);
    //    maybeTriggerOutputFileSwitch();
  }

  finalizeEventEnabled_ = false;
  eventException_ = false;
}

void
art::EventProcessor::incrementInputFileNumber()
{
  endPathExecutor_->incrementInputFileNumber();
}

void
art::EventProcessor::recordOutputClosureRequests(Boundary const b)
{
  endPathExecutor_->recordOutputClosureRequests(b);
}

bool
art::EventProcessor::outputsToOpen() const
{
  return endPathExecutor_->outputsToOpen();
}

bool
art::EventProcessor::outputsToClose() const
{
  return endPathExecutor_->outputsToClose();
}

bool
art::EventProcessor::someOutputsOpen() const
{
  return endPathExecutor_->someOutputsOpen();
}

bool
art::EventProcessor::shouldWeStop() const
{
  FDEBUG(1) << spaces(8) << "shouldWeStop\n";
  if (shouldWeStop_) { return true; }
  return endPathExecutor_->terminate();
}

bool
art::EventProcessor::alreadyHandlingException() const
{
  return alreadyHandlingException_;
}

bool
art::EventProcessor::setTriggerPathEnabled(std::string const& name, bool const enable)
{
  return schedule_->setTriggerPathEnabled(name, enable);
}

bool
art::EventProcessor::setEndPathModuleEnabled(std::string const& label, bool const enable)
{
  return endPathExecutor_->setEndPathModuleEnabled(label, enable);
}

void
art::EventProcessor::servicesActivate_(ServiceToken const st)
{
  servicesSentry_ = std::make_unique<ServiceRegistry::Operate>(st);
}

void
art::EventProcessor::servicesDeactivate_()
{
  servicesSentry_.reset();
}

void
art::EventProcessor::terminateMachine_()
{
  assert(machine_);
  // if (!machine_->terminated()) {
  //   //    machine_->process_event(statemachine::Stop());
  // }
  // else {
  //   FDEBUG(2) << "EventProcessor::terminateMachine_: The state machine was already terminated \n";
  // }
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
