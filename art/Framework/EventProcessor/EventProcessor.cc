#include "art/Framework/EventProcessor/EventProcessor.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Breakpoints.h"
#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceFactory.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/EventProcessor/detail/writeSummary.h"
#include "art/Framework/IO/Root/InitRootHandlers.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Framework/Services/System/FloatingPointControl.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/ProcessConfigurationRegistry.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Utilities/CPCSentry.h"
#include "art/Utilities/CurrentProcessingContext.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/Transition.h"
#include "art/Utilities/UnixSignalHandlers.h"
#include "art/Utilities/bold_fontify.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ParentageRegistry.h"
#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/types/detail/validationException.h"
#include "hep_concurrency/RecursiveMutex.h"
#include "hep_concurrency/tsan.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "tbb/task.h"
#include "tbb/task_arena.h"

#include "TError.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <exception>
#include <functional>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace hep::concurrency;
using namespace std;
using namespace string_literals;
using fhicl::ParameterSet;

namespace art {

  EventProcessor::~EventProcessor()
  {
    ANNOTATE_THREAD_IGNORE_BEGIN;
    delete deferredExceptionPtr_.load();
    deferredExceptionPtr_ = nullptr;
    for (auto val : *eventPrincipal_.load()) {
      delete val;
    }
    delete eventPrincipal_.load();
    eventPrincipal_ = nullptr;
    delete subRunPrincipal_.load();
    subRunPrincipal_ = nullptr;
    delete runPrincipal_.load();
    runPrincipal_ = nullptr;
    delete fb_.load();
    fb_ = nullptr;
    delete endPathExecutor_.load();
    endPathExecutor_ = nullptr;
    delete schedule_.load();
    schedule_ = nullptr;
    delete input_.load();
    input_ = nullptr;
    delete scheduler_.load();
    scheduler_ = nullptr;
    delete pathManager_.load();
    pathManager_ = nullptr;
    delete servicesManager_.load();
    servicesManager_ = nullptr;
    delete psSignals_.load();
    psSignals_ = nullptr;
    delete producedProductLookupTables_.load();
    producedProductLookupTables_ = nullptr;
    delete producedProductDescriptions_.load();
    producedProductDescriptions_ = nullptr;
    delete outputCallbacks_.load();
    outputCallbacks_ = nullptr;
    delete mfStatusUpdater_.load();
    mfStatusUpdater_ = nullptr;
    delete actReg_.load();
    actReg_ = nullptr;
    delete timer_.load();
    timer_ = nullptr;
    delete ec_.load();
    ec_ = nullptr;
    ParentageRegistry::instance(true);
    ProcessConfigurationRegistry::instance(true);
    ProcessHistoryRegistry::instance(true);
    SharedResourcesRegistry::instance(true);
    SetErrorHandler(DefaultErrorHandler);
    TypeID::shutdown();
    ANNOTATE_THREAD_IGNORE_END;
  }

  EventProcessor::EventProcessor(ParameterSet const& pset)
  {
    TypeID::startup();
    auto services_pset = pset.get<ParameterSet>("services");
    auto const scheduler_pset = services_pset.get<ParameterSet>("scheduler");
    {
      // FIXME: Signals and threads require more effort than this!  A
      //        signal is delivered to only one thread, and which
      //        thread is left up to the implementation to decide. To
      //        get control we must block all signals in the main
      //        thread, create a new thread which will handle the
      //        signals we want to handle, unblock the signals in that
      //        thread only, and have it use sigwaitinfo() to suspend
      //        itselt and wait for those signals.
      setupSignals(scheduler_pset.get<bool>("enableSigInt", true));
      if (scheduler_pset.get<bool>("unloadRootSigHandler", true)) {
        unloadRootSigHandler();
      }
      setRootErrorHandler(
        scheduler_pset.get<bool>("resetRootErrHandler", true));
      completeRootHandlers();
    }
    ParentageRegistry::instance();
    ProcessConfigurationRegistry::instance();
    ProcessHistoryRegistry::instance();
    nextLevel_ = Level::ReadyToAdvance;
    ec_ = new detail::ExceptionCollector{};
    timer_ = new cet::cpu_timer{};
    beginRunCalled_ = false;
    beginSubRunCalled_ = false;
    finalizeRunEnabled_ = true;
    finalizeSubRunEnabled_ = true;
    actReg_ = new ActivityRegistry{};
    mfStatusUpdater_ = new MFStatusUpdater{*actReg_.load()};
    outputCallbacks_ = new UpdateOutputCallbacks{};
    producedProductDescriptions_ = new ProductDescriptions{};
    producedProductLookupTables_ = nullptr;
    psSignals_ = new ProducingServiceSignals{};
    {
      auto const fpcPSet =
        services_pset.get<ParameterSet>("FloatingPointControl", {});
      services_pset.erase("FloatingPointControl");
      services_pset.erase("message");
      services_pset.erase("scheduler");
      servicesManager_ =
        new ServicesManager{move(services_pset), *actReg_.load()};
      servicesManager_.load()->addSystemService<FloatingPointControl>(
        fpcPSet, *actReg_.load());
      ServiceRegistry::instance().setManager(servicesManager_.load());
    }
    // We do this late because the floating point control word, signal
    // masks, etc., are per-thread and inherited from the master
    // thread, so we want to allow system services, user services, and
    // modules to configure these things in their constructors before
    // we let tbb create any threads. This means they cannot use tbb
    // in their constructors, instead they must use the beginJob
    // callout.
    scheduler_ = new Scheduler{scheduler_pset};
    auto const nschedules = scheduler_.load()->num_schedules();
    auto const nthreads = scheduler_.load()->num_threads();
    auto const& processName{pset.get<string>("process_name")};
    Globals::instance()->setProcessName(processName);
    {
      ostringstream msg;
      msg << "nschedules: " << nschedules << " nthreads: " << nthreads;
      TDEBUG_FUNC_MSG(5, "EventProcessor::EventProcessor", msg.str());
    }
    pathManager_ = new PathManager{pset,
                                   *outputCallbacks_.load(),
                                   *producedProductDescriptions_.load(),
                                   scheduler_.load()->actionTable(),
                                   *actReg_.load()};
    ParameterSet triggerPSet;
    triggerPSet.put("trigger_paths", pathManager_.load()->triggerPathNames());
    /*auto const& triggerPSetID =*/fhicl::ParameterSetRegistry::put(
      triggerPSet);
    Globals::instance()->setTriggerPSet(triggerPSet);
    Globals::instance()->setTriggerPathNames(
      pathManager_.load()->triggerPathNames());
    input_ = nullptr;
    schedule_ = new PerScheduleContainer<Schedule>{};
    endPathExecutor_ = nullptr;
    fb_ = nullptr;
    runPrincipal_ = nullptr;
    subRunPrincipal_ = nullptr;
    eventPrincipal_ = new PerScheduleContainer<EventPrincipal*>{};
    handleEmptyRuns_ = scheduler_.load()->handleEmptyRuns();
    handleEmptySubRuns_ = scheduler_.load()->handleEmptySubRuns();
    deferredExceptionPtrIsSet_ = false;
    deferredExceptionPtr_ = new exception_ptr{};
    firstEvent_ = true;
    fileSwitchInProgress_ = false;
    eventPrincipal_.load()->expand_to_num_schedules();
    for (auto si = ScheduleID::first();
         si < ScheduleID{static_cast<ScheduleID::size_type>(nschedules)};
         si = si.next()) {
      ANNOTATE_BENIGN_RACE_SIZED(&(*eventPrincipal_.load())[si],
                                 sizeof(EventPrincipal*),
                                 "EventPrincipal ptr");
    }
    auto const errorOnMissingConsumes = scheduler_.load()->errorOnMissingConsumes();
    ConsumesInfo::instance()->setRequireConsumes(errorOnMissingConsumes);
    {
      auto const& physicsPSet = pset.get<ParameterSet>("physics", {});
      servicesManager_.load()->addSystemService<TriggerNamesService>(
        pathManager_.load()->triggerPathNames(),
        processName,
        triggerPSet,
        physicsPSet);
    }
    // We have delayed creating the service module instances, now actually
    // create them.
    servicesManager_.load()->forceCreation();
    ServiceHandle<FileCatalogMetadata> {}
    ->addMetadataString("process_name", processName);
    {
      // Now that the service module instances have been created
      // we can set the callbacks, set the module description, and
      // register the products for each service module instance.
      ProcessConfiguration const pc{
        processName, pset.id(), getReleaseVersion()};
      servicesManager_.load()->registerProducts(
        *producedProductDescriptions_.load(), *psSignals_.load(), pc);
    }
    pathManager_.load()->createModulesAndWorkers();
    endPathExecutor_ = new EndPathExecutor{*pathManager_.load(),
                                           scheduler_.load()->actionTable(),
                                           *actReg_.load(),
                                           *outputCallbacks_.load()};
    for (auto I = ScheduleID::first();
         I < ScheduleID{static_cast<ScheduleID::size_type>(nschedules)};
         I = I.next()) {
      schedule_.load()->emplace_back(I,
                                     *pathManager_.load(),
                                     processName,
                                     pset,
                                     triggerPSet,
                                     *outputCallbacks_.load(),
                                     *producedProductDescriptions_.load(),
                                     scheduler_.load()->actionTable(),
                                     *actReg_.load());
    }
    FDEBUG(2) << pset.to_string() << endl;
    // The input source must be created after the end path executor
    // because the end path executor registers a callback that must
    // be invoked after the first input file is opened.
    {
      ParameterSet main_input;
      main_input.put("module_type", "EmptyEvent");
      main_input.put("module_label", "source");
      main_input.put("maxEvents", -1);
      if (!pset.get_if_present("source", main_input)) {
        mf::LogInfo("EventProcessorSourceConfig")
          << "Could not find a source configuration: using default.";
      }
      ModuleDescription const md{
        main_input.id(),
        main_input.get<string>("module_type"),
        main_input.get<string>("module_label"),
        static_cast<int>(ModuleThreadingType::LEGACY),
        ProcessConfiguration{processName, pset.id(), getReleaseVersion()}};
      InputSourceDescription isd{md, *outputCallbacks_.load(), *actReg_.load()};
      try {
        input_ = InputSourceFactory::make(main_input, isd).release();
      }
      catch (fhicl::detail::validationException const& e) {
        throw Exception(errors::Configuration)
          << "\n\nModule label: " << detail::bold_fontify(md.moduleLabel())
          << "\nmodule_type : " << detail::bold_fontify(md.moduleName())
          << "\n\n"
          << e.what();
      }
      catch (Exception const& x) {
        if (x.categoryCode() == errors::Configuration) {
          throw Exception(errors::Configuration, "FailedInputSource")
            << "Configuration of main input source has failed\n"
            << x;
        }
        throw;
      }
      catch (cet::exception const& x) {
        throw Exception(errors::Configuration, "FailedInputSource")
          << "Configuration of main input source has failed\n"
          << x;
      }
      catch (...) {
        throw;
      }
    }
    actReg_.load()->sPostSourceConstruction.invoke(
      input_.load()->moduleDescription());
    // Create product tables used for product retrieval within modules.
    producedProductLookupTables_ =
      new ProductTables{*producedProductDescriptions_.load()};
    outputCallbacks_.load()->invoke(*producedProductLookupTables_.load());
  }

  void
  EventProcessor::invokePostBeginJobWorkers_()
  {
    // Need to convert multiple lists of workers into a long list that
    // the postBeginJobWorkers callbacks can understand.
    vector<Worker*> allWorkers;
    {
      auto const& workers =
        pathManager_.load()->triggerPathsInfo(ScheduleID::first()).workers();
      for_each(workers.cbegin(),
               workers.cend(),
               [&allWorkers](auto const& label_And_worker) {
                 allWorkers.push_back(label_And_worker.second);
               });
    }
    {
      auto const& workers = pathManager_.load()->endPathInfo().workers();
      for_each(workers.cbegin(),
               workers.cend(),
               [&allWorkers](auto const& label_And_worker) {
                 allWorkers.push_back(label_And_worker.second);
               });
    }
    actReg_.load()->sPostBeginJobWorkers.invoke(input_.load(), allWorkers);
  }

  //================================================================
  // Event-loop infrastructure

  template <Level L>
  bool
  EventProcessor::levelsToProcess()
  {
    if (nextLevel_.load() == Level::ReadyToAdvance) {
      nextLevel_ = advanceItemType();
      // Consider reading right here?
    }
    if (nextLevel_.load() == L) {
      nextLevel_ = Level::ReadyToAdvance;
      if (endPathExecutor_.load()->outputsToClose()) {
        setOutputFileStatus(OutputFileStatus::Switching);
        finalizeContainingLevels<L>();
        closeSomeOutputFiles();
      }
      return true;
    } else if (nextLevel_.load() < L) {
      return false;
    } else if (nextLevel_.load() == highest_level()) {
      return false;
    }
    throw Exception{errors::LogicError} << "Incorrect level hierarchy.";
  }

  // Specializations for process function template

  template <>
  inline void
  EventProcessor::begin<Level::Job>()
  {
    timer_.load()->start();
    beginJob();
  }

  template <>
  inline void
  EventProcessor::begin<Level::InputFile>()
  {
    openInputFile();
  }

  template <>
  void
  EventProcessor::begin<Level::Run>()
  {
    finalizeRunEnabled_ = true;
    readRun();
    if (handleEmptyRuns_.load()) {
      beginRun();
    }
  }

  template <>
  void
  EventProcessor::begin<Level::SubRun>()
  {
    finalizeSubRunEnabled_ = true;
    assert(runPrincipal_.load());
    assert(runPrincipal_.load()->runID().isValid());
    readSubRun();
    if (handleEmptySubRuns_.load()) {
      beginRunIfNotDoneAlready();
      beginSubRun();
    }
  }

  template <>
  void
  EventProcessor::finalize<Level::SubRun>()
  {
    assert(subRunPrincipal_.load());
    if (!finalizeSubRunEnabled_) {
      return;
    }
    if (subRunPrincipal_.load()->subRunID().isFlush()) {
      return;
    }
    openSomeOutputFiles();
    setSubRunAuxiliaryRangeSetID();
    if (beginSubRunCalled_) {
      endSubRun();
    }
    writeSubRun();
    finalizeSubRunEnabled_ = false;
  }

  template <>
  void
  EventProcessor::finalize<Level::Run>()
  {
    assert(runPrincipal_.load());
    if (!finalizeRunEnabled_) {
      return;
    }
    if (runPrincipal_.load()->runID().isFlush()) {
      return;
    }
    openSomeOutputFiles();
    setRunAuxiliaryRangeSetID();
    if (beginRunCalled_) {
      endRun();
    }
    writeRun();
    finalizeRunEnabled_ = false;
  }

  template <>
  void
  EventProcessor::finalize<Level::InputFile>()
  {
    if (nextLevel_.load() == Level::Job) {
      closeAllFiles();
    } else {
      closeInputFile();
    }
  }

  template <>
  void
  EventProcessor::finalize<Level::Job>()
  {
    endJob();
    timer_.load()->stop();
  }

  template <>
  void
  EventProcessor::finalizeContainingLevels<Level::SubRun>()
  {
    finalize<Level::Run>();
  }

  template <>
  void
  EventProcessor::finalizeContainingLevels<Level::Event>()
  {
    finalize<Level::SubRun>();
    finalize<Level::Run>();
  }

  template <>
  void
  EventProcessor::recordOutputModuleClosureRequests<Level::Run>()
  {
    endPathExecutor_.load()->recordOutputClosureRequests(Granularity::Run);
  }

  template <>
  void
  EventProcessor::recordOutputModuleClosureRequests<Level::SubRun>()
  {
    endPathExecutor_.load()->recordOutputClosureRequests(Granularity::SubRun);
  }

  template <>
  void
  EventProcessor::recordOutputModuleClosureRequests<Level::Event>()
  {
    endPathExecutor_.load()->recordOutputClosureRequests(Granularity::Event);
  }

  class ProcessAllEventsFunctor {
  public:
    ProcessAllEventsFunctor(EventProcessor* evp,
                            tbb::task* eventLoopTask,
                            ScheduleID const si)
      : evp_(evp), eventLoopTask_(eventLoopTask), si_(si)
    {}
    void
    operator()(exception_ptr const* ex)
    {
      evp_->processAllEventsTask(eventLoopTask_, si_, ex);
    }

  private:
    EventProcessor* evp_;
    tbb::task* eventLoopTask_;
    ScheduleID const si_;
  };

  void
  EventProcessor::processAllEventsTask(tbb::task* eventLoopTask,
                                       ScheduleID const si,
                                       exception_ptr const*)
  {
    TDEBUG_BEGIN_TASK_SI(4, "processAllEventsTask", si);
    INTENTIONAL_DATA_RACE(DR_EP_PROCESS_ALL_EVENTS_TASK);
    processAllEventsAsync(eventLoopTask, si);
    TDEBUG_END_TASK_SI(4, "processAllEventsTask", si);
  }

  template <>
  void
  EventProcessor::process<most_deeply_nested_level()>()
  {
    auto const nschedules = Globals::instance()->nschedules();
    if ((shutdown_flag > 0) || !ec_.load()->empty()) {
      return;
    }
    // Note: This loop is to allow output file switching to
    // happen in the main thread.
    firstEvent_ = true;
    bool done = false;
    while (!done) {
      beginRunIfNotDoneAlready();
      beginSubRunIfNotDoneAlready();
      struct Waiter : public tbb::task {
        tbb::task*
        execute()
        {
          return nullptr;
        }
      };
      auto eventLoopTask = new (tbb::task::allocate_root()) Waiter;
      eventLoopTask->set_ref_count(nschedules + 1);
      tbb::task_list schedule_heads;
      for (auto si = ScheduleID::first();
           si < ScheduleID{static_cast<ScheduleID::size_type>(nschedules)};
           si = si.next()) {
        auto processAllEventsTask =
          make_waiting_task(eventLoopTask->allocate_child(),
                            ProcessAllEventsFunctor{this, eventLoopTask, si});
        schedule_heads.push_back(*processAllEventsTask);
      }
      eventLoopTask->spawn_and_wait_for_all(schedule_heads);
      tbb::task::destroy(*eventLoopTask);
      // If anything bad happened during event processing,
      // let the user know.
      if (deferredExceptionPtrIsSet_.load()) {
        rethrow_exception(*deferredExceptionPtr_.load());
      }
      auto do_switch = fileSwitchInProgress_.load();
      if (!do_switch) {
        done = true;
        continue;
      }
      setOutputFileStatus(OutputFileStatus::Switching);
      finalizeContainingLevels<most_deeply_nested_level()>();
      respondToCloseOutputFiles();
      {
        endPathExecutor_.load()->closeSomeOutputFiles();
        FDEBUG(1) << string(8, ' ') << "closeSomeOutputFiles\n";
      }
      // We started the switch after advancing the file index
      // iterator, we must make sure that we read that event
      // before advancing the iterator again.
      firstEvent_ = true;
      fileSwitchInProgress_ = false;
    }
  }

  class ReadAndProcessEventFunctor {
  public:
    ReadAndProcessEventFunctor(EventProcessor* evp,
                               tbb::task* eventLoopTask,
                               ScheduleID const si)
      : evp_(evp), eventLoopTask_(eventLoopTask), si_(si)
    {}
    void
    operator()(exception_ptr const* ex)
    {
      INTENTIONAL_DATA_RACE(DR_EP_READ_AND_PROCESS_EVENT_FUNCTOR);
      evp_->readAndProcessEventTask(eventLoopTask_, si_, ex);
    }

  private:
    EventProcessor* evp_;
    tbb::task* eventLoopTask_;
    ScheduleID const si_;
  };

  void
  EventProcessor::readAndProcessEventTask(tbb::task* eventLoopTask,
                                          ScheduleID const si,
                                          exception_ptr const*)
  {
    // Note: When we come here our parent is the eventLoop task.
    TDEBUG_BEGIN_TASK_SI(4, "readAndProcessEventTask", si);
    INTENTIONAL_DATA_RACE(DR_EP_READ_AND_PROCESS_EVENT_TASK);
    try {
      // Note: We pass eventLoopTask here to keep the thread sanitizer happy.
      readAndProcessAsync(eventLoopTask, si);
    }
    catch (...) {
      // Event processing threw an exception.
      // Use a thread-safe, one-way trapdoor pattern to notify
      // the main thread.
      bool expected = false;
      if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected, true)) {
        // Put the exception where the main thread can get at it.
        *deferredExceptionPtr_.load() = current_exception();
      }
      // And then end this task, terminating event processing.
      TDEBUG_END_TASK_SI_ERR(4,
                             "readAndProcessEventTask",
                             si,
                             "terminate event loop because of EXCEPTION");
      return;
    }
    // And then end this task, which does not terminate event
    // processing because our parent is the nullptr because
    // we transferred it to another task.
    TDEBUG_END_TASK_SI(4, "readAndProcessEventTask", si);
    return;
  }

  // This is the event loop (also known as the schedule head).
  // It makes a continuation task (the readAndProcessEventTask)
  // which reads and processes a single event, creates itself
  // again as a continuation task, and then exits. We are passed
  // the EventLoopTask here to keep the thread sanitizer happy.
  void
  EventProcessor::processAllEventsAsync(tbb::task* eventLoopTask,
                                        ScheduleID const si)
  {
    // Note: We are part of the processAllEventsTask (schedule head task),
    // and our parent is the eventLoopTask.
    TDEBUG_BEGIN_FUNC_SI(4, "EventProcessor::processAllEventsAsync", si);
    INTENTIONAL_DATA_RACE(DR_EP_PROCESS_ALL_EVENTS_ASYNC);
    // Create a continuation task that has the EventLoopTask as parent,
    // and reset our parent to the nullptr at the same time, which means when
    // we end we do not decrement the ref count of the EventLoopTask and our
    // continuation runs, not the EventLoopTask.
    auto readAndProcessEventTask =
      make_waiting_task(tbb::task::self().allocate_continuation(),
                        ReadAndProcessEventFunctor{this, eventLoopTask, si});
    // Push the readAndProcessEventTask onto the end of this
    // thread's tbb scheduling dequeue.
    tbb::task::spawn(*readAndProcessEventTask);
    // And end this task, which does not terminate event processing
    // because our parent is the nullptr because we transferred it
    // to the readAndProcessEventTask with allocate_continuation().
    TDEBUG_END_FUNC_SI(4, "EventProcessor::processAllEventsAsync", si);
    // Note: We end and our continuation, the readAndProcessEventTask begins
    // on this same thread because it is the first task on this thread's
    // tbb scheduling deque, and we have no parent, and we return the nullptr.
  }

  // This function is executed as part of the readAndProcessEvent task,
  // our parent task is the EventLoopTask. Here we advance to the next item
  // in the file index, end event processing if it is not an event, or if
  // the user has requested a shutdown, read the event, and then call another
  // function to do the processing.
  void
  EventProcessor::readAndProcessAsync(tbb::task* EventLoopTask,
                                      ScheduleID const si)
  {
    // Note: We are part of the readAndProcessEventTask (schedule head task),
    // and our parent task is the EventLoopTask.
    TDEBUG_BEGIN_FUNC_SI(4, "EventProcessor::readAndProcessAsync", si);
    INTENTIONAL_DATA_RACE(DR_EP_READ_AND_PROCESS_ASYNC);
    // Note: shutdown_flag is a extern global atomic int in
    // art/art/Utilities/UnixSignalHandlers.cc
    if (shutdown_flag) {
      // User called for a clean shutdown using a signal or ctrl-c,
      // end event processing and this task.
      TDEBUG_END_FUNC_SI_ERR(4, "readAndProcessAsync", si, "CLEAN SHUTDOWN");
      return;
    }
    {
      endPathExecutor_.load()->check();
    }
    // The item type advance and the event read must be
    // done with the input source lock held, however the
    // event processing should not be.
    {
      input::RootMutexSentry lock_input;
      auto do_switch = fileSwitchInProgress_.load();
      if (do_switch) {
        // We must avoid advancing the iterator after
        // a schedule has noticed it is time to switch files.
        // After the switch, we will need to set firstEvent_
        // true so that the first schedule that resumes after
        // the switch actually reads the event that the first
        // schedule which noticed we needed a switch had advanced
        // the iterator to.
        // Note: We still have the problem that because the
        // schedules do not read events at the same time the file
        // switch point can be up to #nschedules-1 ahead of where
        // it would have been if there was only one schedule.
        // If we are switching output files every event in an
        // attempt to create single event files, this really does
        // not work out too well.
        TDEBUG_END_FUNC_SI_ERR(
          4, "EventProcessor::readAndProcessAsync", si, "FILE SWITCH");
        return;
      }
      //
      //  Check the file index for what comes next and exit
      //  this task if it is not an event, or if the user
      //  has asynchronously requested a shutdown.
      //
      auto expected = true;
      if (firstEvent_.compare_exchange_strong(expected, false)) {
        // Do not advance the item type on the first event.
      } else {
        // Do the advance item type.
        if (nextLevel_.load() == Level::ReadyToAdvance) {
          // See what the next item is.
          TDEBUG_FUNC_SI_MSG(5,
                             "EventProcessor::readAndProcessAsync",
                             si,
                             "Calling advanceItemType()");
          nextLevel_ = advanceItemType();
        }
        if ((nextLevel_.load() < most_deeply_nested_level()) ||
            (nextLevel_.load() == highest_level())) {
          // We are popping up, end event processing and this task.
          TDEBUG_END_FUNC_SI_ERR(
            4, "EventProcessor::readAndProcessAsync", si, "END OF SUBRUN");
          return;
        }
        if (nextLevel_.load() != most_deeply_nested_level()) {
          // Error: incorrect level hierarchy
          TDEBUG_END_FUNC_SI_ERR(
            4, "EventProcessor::readAndProcessAsync", si, "BAD HIERARCHY");
          throw Exception{errors::LogicError} << "Incorrect level hierarchy.";
        }
        nextLevel_ = Level::ReadyToAdvance;
        // At this point we have determined that we are going to read an event
        // and we must do that before dropping the lock on the input source
        // which is what is protecting us against a double-advance caused by
        // a different schedule.
        auto mustClose = endPathExecutor_.load()->outputsToClose();
        if (mustClose) {
          fileSwitchInProgress_ = true;
          TDEBUG_END_FUNC_SI_ERR(4,
                                 "EventProcessor::readAndProcessAsync",
                                 si,
                                 "FILE SWITCH INITIATED");
          return;
        }
      }
      //
      //  Now we can read the event from the source.
      //
      assert(subRunPrincipal_.load());
      // FIXME: This assert is causing a data race!
      // assert(subRunPrincipal_.load()->subRunID().isValid());
      {
        CurrentProcessingContext cpc{si, nullptr, -1, false};
        detail::CPCSentry sentry{cpc};
        actReg_.load()->sPreSourceEvent.invoke();
      }
      TDEBUG_FUNC_SI_MSG(5,
                         "readAndProcessAsync",
                         si,
                         "Calling input_->readEvent(subRunPrincipal_.load())");
      (*eventPrincipal_.load())[si] =
        input_.load()->readEvent(subRunPrincipal_.load()).release();
      assert((*eventPrincipal_.load())[si]);
      // The intended behavior here is that the producing services which are
      // called during the sPostReadEvent cannot see each others put products.
      // We enforce this by creating the groups for the produced products, but
      // do not allow the lookups to find them until after the callbacks have
      // run.
      (*eventPrincipal_.load())[si]->createGroupsForProducedProducts(
        *producedProductLookupTables_.load());
      psSignals_.load()->sPostReadEvent.invoke(*(*eventPrincipal_.load())[si]);
      (*eventPrincipal_.load())[si]->enableLookupOfProducedProducts(
        *producedProductLookupTables_.load());
      {
        CurrentProcessingContext cpc{si, nullptr, -1, false};
        detail::CPCSentry sentry{cpc};
        Event const e{*(*eventPrincipal_.load())[si], ModuleDescription{}};
        actReg_.load()->sPostSourceEvent.invoke(e);
      }
      FDEBUG(1) << string(8, ' ') << "readEvent...................("
                << (*eventPrincipal_.load())[si]->eventID() << ")\n";
      // Now we drop the input source lock by exiting the guarded scope.
    }
    INTENTIONAL_DATA_RACE(
      DR_EP_READ_AND_PROCESS_ASYNC_AFTER_INPUT_MUTEX_UNLOCK);
    if ((*eventPrincipal_.load())[si]->eventID().isFlush()) {
      // No processing to do, start next event handling task,
      // transferring our parent task (EventLoopTask) to it,
      // and exit this task.
      processAllEventsAsync(EventLoopTask, si);
      TDEBUG_END_FUNC_SI_ERR(
        4, "EventProcessor::readAndProcessAsync", si, "FLUSH EVENT");
      return;
    }
    // Now process the event.
    processEventAsync(EventLoopTask, si);
    // And end this task, which does not terminate event processing
    // because our parent is the nullptr because we transferred it
    // to the endPathTask.
    TDEBUG_END_FUNC_SI(4, "EventProcessor::readAndProcessAsync", si);
  }

  class EndPathFunctor {
  public:
    EndPathFunctor(EventProcessor* evp,
                   tbb::task* eventLoopTask,
                   ScheduleID const si)
      : evp_{evp}, eventLoopTask_{eventLoopTask}, si_{si}
    {}
    void
    operator()(exception_ptr const* ex)
    {
      evp_->endPathTask(eventLoopTask_, si_, ex);
    }

  private:
    EventProcessor* evp_;
    tbb::task* eventLoopTask_;
    ScheduleID const si_;
  };

  void
  EventProcessor::endPathTask(tbb::task* eventLoopTask,
                              ScheduleID const si,
                              exception_ptr const* ex)
  {
    // Note: When we start our parent is the eventLoopTask.
    TDEBUG_BEGIN_TASK_SI(4, "endPathTask", si);
    INTENTIONAL_DATA_RACE(DR_EP_END_PATH_TASK);
    if (ex != nullptr) {
      try {
        rethrow_exception(*ex);
      }
      catch (cet::exception& e) {
        if (scheduler_.load()->actionTable().find(e.root_cause()) !=
            actions::IgnoreCompletely) {
          auto ex_ptr = make_exception_ptr(
            Exception{errors::EventProcessorFailure,
                      "EventProcessor: an exception occurred during current "
                      "event processing",
                      e});
          // Use a thread-safe, one-way trapdoor pattern to notify the
          // main thread of the exception.
          bool expected = false;
          if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected,
                                                                 true)) {
            // Put the exception where the main thread can get at it.
            *deferredExceptionPtr_.load() = ex_ptr;
          }
          // And then end this task, terminating event processing.
          TDEBUG_END_TASK_SI_ERR(
            4, "endPathTask", si, "terminate event loop because of EXCEPTION");
          return;
        }
        mf::LogWarning(e.category())
          << "exception being ignored for current event:\n"
          << cet::trim_right_copy(e.what(), " \n");
        // WARNING: We continue processing after the catch blocks!!!
      }
      catch (...) {
        mf::LogError("PassingThrough")
          << "an exception occurred during current event processing\n";
        // Use a thread-safe, one-way trapdoor pattern to notify the main
        // thread of the exception.
        bool expected = false;
        if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected,
                                                               true)) {
          // Put the exception where the main thread can get at it.
          *deferredExceptionPtr_.load() = current_exception();
        }
        // And then end this task, terminating event processing.
        TDEBUG_END_TASK_SI_ERR(
          4, "endPathTask", si, "terminate event loop because of EXCEPTION");
        return;
      }
      // WARNING: We only get here if the trigger paths threw
      // and we are ignoring the exception because of
      // actions::IgnoreCompletely.
    }
    processEndPathAsync(eventLoopTask, si);
    // And end this task, which does not terminate event
    // processing because our parent is the nullptr because
    // it got transferred to the next process event task.
    TDEBUG_END_TASK_SI(4, "endPathTask", si);
  }

  // This function is a continuation of the body of the readAndProcessEvent
  // task. Here we call down to Schedule to do the trigger path processing,
  // passing it a waiting task which will do the end path processing, finalize
  // the event, and start the next read and process event task.  Note that
  // Schedule will spawn a task to process each of the trigger paths, and then
  // when they are finished, insert the trigger results, and then spawn the
  // waiting task we gave it to do the end path processing, write the event, and
  // then start the next event processing task.
  void
  EventProcessor::processEventAsync(tbb::task* eventLoopTask,
                                    ScheduleID const si)
  {
    // Note: We are part of the readAndProcessEventTask (schedule head task),
    // and our parent task is the EventLoopTask.
    TDEBUG_BEGIN_FUNC_SI(4, "EventProcessor::processEventAsync", si);
    INTENTIONAL_DATA_RACE(DR_EP_PROCESS_EVENT_ASYNC);
    assert((*eventPrincipal_.load())[si]);
    assert(!(*eventPrincipal_.load())[si]->eventID().isFlush());
    try {
      // Make the end path processing task, make its parent
      // the EventLoopTask, and set our parent to the nullptr
      // so we can exit without ending event processing.
      auto endPathTask =
        make_waiting_task(tbb::task::self().allocate_continuation(),
                          EndPathFunctor{this, eventLoopTask, si});
      {
        Event const ev{*(*eventPrincipal_.load())[si], ModuleDescription{}};
        CurrentProcessingContext cpc{si, nullptr, -1, false};
        detail::CPCSentry sentry{cpc};
        actReg_.load()->sPreProcessEvent.invoke(ev);
      }
      // Start the trigger paths running.  When they finish
      // they will spawn the endPathTask which will run the
      // end path, write the event, and start the next event
      // processing task.
      INTENTIONAL_DATA_RACE(
        DR_EP_PROCESS_EVENT_ASYNC_JUST_BEFORE_SCHEDULE_PROCESS_EVENT);
      (*schedule_.load())[si].process_event(
        endPathTask, eventLoopTask, *(*eventPrincipal_.load())[si], si);
      // Once the trigger paths are running we are done, exit this task,
      // which does not end event processing because our parent is the
      // nullptr because we transferred it to the endPathTask above.
      TDEBUG_END_FUNC_SI(4, "EventProcessor::processEventAsync", si);
      return;
    }
    catch (cet::exception& e) {
      if (scheduler_.load()->actionTable().find(e.root_cause()) !=
          actions::IgnoreCompletely) {
        auto ex_ptr =
          make_exception_ptr(Exception{errors::EventProcessorFailure,
                                       "EventProcessor: an exception "
                                       "occurred during current event "
                                       "processing",
                                       e});
        // Use a thread-safe, one-way trapdoor pattern to notify the main
        // thread of the exception.
        bool expected = false;
        if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected,
                                                               true)) {
          // Put the exception where the main thread can get at it.
          *deferredExceptionPtr_.load() = ex_ptr;
        }
        // And then end this task, terminating event processing.
        ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                     sizeof(tbb::internal::task_prefix),
                                   sizeof(tbb::task) +
                                     sizeof(tbb::internal::task_prefix),
                                   "tbb::task");
        tbb::task::self().set_parent(eventLoopTask);
        TDEBUG_END_TASK_SI_ERR(4,
                               "endPathTask",
                               si,
                               "terminate event loop "
                               "because of EXCEPTION");
        return;
      }
      mf::LogWarning(e.category())
        << "exception being ignored for current event:\n"
        << cet::trim_right_copy(e.what(), " \n");
      // Do this in case we already gave our parent to
      // to endPathTask.
      ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                   sizeof(tbb::internal::task_prefix),
                                 sizeof(tbb::task) +
                                   sizeof(tbb::internal::task_prefix),
                                 "tbb::task");
      tbb::task::self().set_parent(eventLoopTask);
      // WARNING: We continue processing after the catch blocks!!!
    }
    catch (...) {
      mf::LogError("PassingThrough")
        << "an exception occurred during current event processing\n";
      // Use a thread-safe, one-way trapdoor pattern to notify the main thread
      // of the exception.
      bool expected = false;
      if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected, true)) {
        // Put the exception where the main thread can get at it.
        *deferredExceptionPtr_.load() = current_exception();
      }
      // Do this in case we already gave our parent to
      // to endPathTask.
      ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                   sizeof(tbb::internal::task_prefix),
                                 sizeof(tbb::task) +
                                   sizeof(tbb::internal::task_prefix),
                                 "tbb::task");
      tbb::task::self().set_parent(eventLoopTask);
      // And then end this task, terminating event processing.
      TDEBUG_END_TASK_SI_ERR(4,
                             "endPathTask",
                             si,
                             "terminate event loop "
                             "because of EXCEPTION");
      return;
    }
    // WARNING: The only way to get here is if starting the trigger path
    // processing threw and actions::IgnoreCompletely is set!
    finishEventAsync(eventLoopTask, si);
    // And then end this task, which does not end event
    // processing because we finishEvent transferred our
    // parent to the next event processing task.
    TDEBUG_END_FUNC_SI(4, "EventProcessor::processEventAsync", si);
  }

  class EndPathRunnerFunctor {
  public:
    EndPathRunnerFunctor(EventProcessor* evp,
                         ScheduleID const si,
                         tbb::task* eventLoopTask)
      : evp_{evp}, si_{si}, eventLoopTask_{eventLoopTask}
    {}
    void
    operator()() const
    {
      evp_->endPathRunnerTask(si_, eventLoopTask_);
    }

  private:
    EventProcessor* evp_;
    ScheduleID const si_;
    tbb::task* eventLoopTask_;
  };

  void
  EventProcessor::endPathRunnerTask(ScheduleID const si,
                                    tbb::task* eventLoopTask)
  {
    TDEBUG_BEGIN_TASK_SI(4, "endPathFunctor", si);
    INTENTIONAL_DATA_RACE(DR_EP_END_PATH_RUNNER_TASK);
    // Arrange it so that we can terminate event
    // processing if we want to.
    ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                 sizeof(tbb::internal::task_prefix),
                               sizeof(tbb::task) +
                                 sizeof(tbb::internal::task_prefix),
                               "tbb::task");
    tbb::task::self().set_parent(eventLoopTask);
    try {
      endPathExecutor_.load()->process_event(*(*eventPrincipal_.load())[si],
                                             si);
    }
    catch (cet::exception& e) {
      // Possible actions: IgnoreCompletely, Rethrow, SkipEvent, FailModule,
      // FailPath
      if (scheduler_.load()->actionTable().find(e.root_cause()) !=
          actions::IgnoreCompletely) {
        // Possible actions: Rethrow, SkipEvent, FailModule, FailPath
        auto ex_ptr = make_exception_ptr(
          Exception{errors::EventProcessorFailure,
                    "EventProcessor: an exception occurred during "
                    "current event processing",
                    e});
        // Use a thread-safe, one-way trapdoor pattern to
        // notify the main thread of the exception.
        bool expected = false;
        if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected,
                                                               true)) {
          // Put the exception where the main thread can get at it.
          *deferredExceptionPtr_.load() = ex_ptr;
        }
        // And then end this task, terminating event processing.
        ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                     sizeof(tbb::internal::task_prefix),
                                   sizeof(tbb::task) +
                                     sizeof(tbb::internal::task_prefix),
                                   "tbb::task");
        tbb::task::self().set_parent(eventLoopTask);
        TDEBUG_END_TASK_SI_ERR(
          4, "endPathFunctor", si, "terminate event loop because of EXCEPTION");
        return;
      }
      // Possible actions: IgnoreCompletely
      mf::LogWarning(e.category())
        << "exception being ignored for current event:\n"
        << cet::trim_right_copy(e.what(), " \n");
      // WARNING: We continue processing after the catch blocks!!!
      // WARNING: The only way to get here is if end path
      // processing threw and we are ignoring the exception
      // because of actions::IgnoreCompletely.
    }
    catch (...) {
      mf::LogError("PassingThrough")
        << "an exception occurred during current event processing\n";
      // Use a thread-safe, one-way trapdoor pattern to
      // notify the main thread of the exception.
      bool expected = false;
      if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected, true)) {
        // Put the exception where the main thread can get at it.
        *deferredExceptionPtr_.load() = current_exception();
      }
      // And then end this task, terminating event processing.
      ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                   sizeof(tbb::internal::task_prefix),
                                 sizeof(tbb::task) +
                                   sizeof(tbb::internal::task_prefix),
                                 "tbb::task");
      tbb::task::self().set_parent(eventLoopTask);
      TDEBUG_END_TASK_SI_ERR(
        4, "endPathFunctor", si, "terminate event loop because of EXCEPTION");
      return;
    }
    {
      Event const ev{*(*eventPrincipal_.load())[si], ModuleDescription{}};
      CurrentProcessingContext cpc{si, nullptr, -1, false};
      detail::CPCSentry sentry{cpc};
      actReg_.load()->sPostProcessEvent.invoke(ev);
    }
    finishEventAsync(eventLoopTask, si);
    // Note that we do not terminate event processing when we end
    // because finishEventAsync has set our parent to the nullptr.
    TDEBUG_END_TASK_SI(4, "endPathFunctor", si);
  }

  // This function is the main body of the Process End Path task, our
  // parent is the eventLoopTask.
  // Here we create a functor which will call down to EndPathExecutor
  // to do the end path processing serially, without using tasks, and
  // then proceed on to write the event, and then start the next event
  // processing task.  This functor makes running the end path and
  // writing the event into a nice package which we then push onto the
  // EndPathExecutor serial task queue.  This ensures that only one
  // event at a time is being processed by the end path.  Essentially
  // the nschedules end when we push onto the queue, and are recreated
  // after the event is written to process the next event.
  void
  EventProcessor::processEndPathAsync(tbb::task* eventLoopTask,
                                      ScheduleID const si)
  {
    // Note: We are part of the endPathTask.
    TDEBUG_BEGIN_FUNC_SI(4, "EventProcessor::processEndPathAsync", si);
    // Arrange it so that we can end the task without
    // terminating event processing.
    ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                 sizeof(tbb::internal::task_prefix),
                               sizeof(tbb::task) +
                                 sizeof(tbb::internal::task_prefix),
                               "tbb::task");
    tbb::task::self().set_parent(nullptr);
    endPathExecutor_.load()->push(
      EndPathRunnerFunctor{this, si, eventLoopTask});
    // Once the end path processing and event finalization processing
    // is queued we are done, exit this task, which does not end event
    // processing because our parent is the nullptr because we transferred
    // it to the endPathFunctor.
    TDEBUG_END_FUNC_SI(4, "EventProcessor::processEndPathAsync", si);
  }

  // This function is a continuation of the Process End Path task, or
  // the error handling for an ignored trigger path exception in the
  // Process All Events task, or an ignored end path exception in the
  // Process End Path task.  Our parent task is the eventLoopTask.
  // We write the event out, spawn the next event processing task, and
  // end the current task.
  void
  EventProcessor::finishEventAsync(tbb::task* eventLoopTask,
                                   ScheduleID const si)
  {
    // Note: We are part of the endPathFunctor.
    TDEBUG_BEGIN_FUNC_SI(4, "EventProcessor::finishEventAsync", si);
    FDEBUG(1) << string(8, ' ') << "processEvent................("
              << (*eventPrincipal_.load())[si]->eventID() << ")\n";
    try {
      // Ask the output workers if they have reached their limits,
      // and if so setup to end the job the next time around the
      // event loop.
      FDEBUG(1) << string(8, ' ') << "shouldWeStop\n";
      TDEBUG_FUNC_SI_MSG(5,
                         "EventProcessor::finishEventAsync",
                         si,
                         "Calling endPathExecutor_->allAtLimit()");
      if (endPathExecutor_.load()->allAtLimit()) {
        // Set to return to the File level.
        nextLevel_ = highest_level();
      }
      // Now we can write the results of processing to the outputs,
      // and delete the event principal.
      assert((*eventPrincipal_.load())[si]);
      auto isFlush = (*eventPrincipal_.load())[si]->eventID().isFlush();
      if (!isFlush) {
        // Possibly open new output files.
        TDEBUG_FUNC_SI_MSG(5,
                           "EventProcessor::finishEventAsync",
                           si,
                           "Calling endPathExecutor_->outputsToOpen()");
        auto toOpen = endPathExecutor_.load()->outputsToOpen();
        if (toOpen) {
          TDEBUG_FUNC_SI_MSG(
            5,
            "EventProcessor::finishEventAsync",
            si,
            "Calling endPathExecutor_->openSomeOutputFiles(*fb_)");
          endPathExecutor_.load()->openSomeOutputFiles(*fb_.load());
          FDEBUG(1) << string(8, ' ') << "openSomeOutputFiles\n";
          respondToOpenOutputFiles();
        }
        assert((*eventPrincipal_.load())[si]);
        assert(!(*eventPrincipal_.load())[si]->eventID().isFlush());
        TDEBUG_FUNC_SI_MSG(5,
                           "EventProcessor::finishEventAsync",
                           si,
                           "Calling endPathExecutor_->writeEvent(si, "
                           "*(*eventPrincipal_.load())[si])");
        // Write the event.
        endPathExecutor_.load()->writeEvent(si, *(*eventPrincipal_.load())[si]);
        FDEBUG(1) << string(8, ' ') << "writeEvent..................("
                  << (*eventPrincipal_.load())[si]->eventID() << ")\n";
        // And delete the event principal.
        delete (*eventPrincipal_.load())[si];
        (*eventPrincipal_.load())[si] = nullptr;
      }
      TDEBUG_FUNC_SI_MSG(5,
                         "EventProcessor::finishEventAsync",
                         si,
                         "Calling endPathExecutor_->"
                         "recordOutputClosureRequests(Granularity::Event)");
      endPathExecutor_.load()->recordOutputClosureRequests(Granularity::Event);
    }
    catch (cet::exception& e) {
      if (scheduler_.load()->actionTable().find(e.root_cause()) !=
          actions::IgnoreCompletely) {
        auto ex_ptr = make_exception_ptr(
          Exception{errors::EventProcessorFailure,
                    "EventProcessor: an exception occurred during "
                    "current event processing",
                    e});
        // Use a thread-safe, one-way trapdoor pattern to notify the
        // main thread of the exception.
        bool expected = false;
        if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected,
                                                               true)) {
          // Put the exception where the main thread can get at it.
          *deferredExceptionPtr_.load() = ex_ptr;
        }
        // And then end this task, terminating event processing.
        ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                     sizeof(tbb::internal::task_prefix),
                                   sizeof(tbb::task) +
                                     sizeof(tbb::internal::task_prefix),
                                   "tbb::task");
        tbb::task::self().set_parent(eventLoopTask);
        TDEBUG_END_FUNC_SI_ERR(
          4, "EventProcessor::finishEventAsync", si, "EXCEPTION");
        return;
      }
      mf::LogWarning(e.category())
        << "exception being ignored for current event:\n"
        << cet::trim_right_copy(e.what(), " \n");
      // WARNING: We continue processing after the catch blocks!!!
    }
    catch (...) {
      mf::LogError("PassingThrough")
        << "an exception occurred during current event processing\n";
      // Use a thread-safe, one-way trapdoor pattern to notify the main thread
      // of the exception.
      bool expected = false;
      if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected, true)) {
        // Put the exception where the main thread can get at it.
        *deferredExceptionPtr_.load() = current_exception();
      }
      // And then end this task, terminating event processing.
      ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                   sizeof(tbb::internal::task_prefix),
                                 sizeof(tbb::task) +
                                   sizeof(tbb::internal::task_prefix),
                                 "tbb::task");
      tbb::task::self().set_parent(eventLoopTask);
      TDEBUG_END_FUNC_SI_ERR(
        4, "EventProcessor::finishEventAsync", si, "EXCEPTION");
      return;
    }
    // Create the next event processing task as a continuation
    // of this task, that is transfer our parent, the eventLoopTask,
    // to it.
    processAllEventsAsync(eventLoopTask, si);
    // And end this task which does not end event loop processing
    // because our parent is the nullptr because we transferred it
    // to the next event processing task.
    TDEBUG_END_FUNC_SI(4, "EventProcessor::finishEventAsync", si);
  }

  template <Level L>
  void
  EventProcessor::process()
  {
    if ((shutdown_flag > 0) || !ec_.load()->empty()) {
      return;
    }
    ec_.load()->call([this] { begin<L>(); });
    while ((shutdown_flag == 0) && ec_.load()->empty() &&
           levelsToProcess<level_down(L)>()) {
      ec_.load()->call([this] { process<level_down(L)>(); });
    }
    ec_.load()->call([this] {
      finalize<L>();
      recordOutputModuleClosureRequests<L>();
    });
  }

  EventProcessor::StatusCode
  EventProcessor::runToCompletion()
  {
    StatusCode returnCode{epSuccess};
    ec_.load()->call([this, &returnCode] {
      process<highest_level()>();
      if (shutdown_flag > 0) {
        returnCode = epSignal;
      }
    });
    if (!ec_.load()->empty()) {
      terminateAbnormally_();
      ec_.load()->rethrow();
    }
    return returnCode;
  }

  Level
  EventProcessor::advanceItemType()
  {
    auto const itemType = input_.load()->nextItemType();
    FDEBUG(1) << string(4, ' ') << "*** nextItemType: " << itemType << " ***\n";
    switch (itemType) {
      case input::IsStop:
        return highest_level();
      case input::IsFile:
        return Level::InputFile;
      case input::IsRun:
        return Level::Run;
      case input::IsSubRun:
        return Level::SubRun;
      case input::IsEvent:
        return Level::Event;
      case input::IsInvalid:
        throw Exception{errors::LogicError}
          << "Invalid next item type presented to the event processor.\n"
          << "Please contact artists@fnal.gov.";
    }
    throw Exception{errors::LogicError}
      << "Unrecognized next item type presented to the event processor.\n"
      << "Please contact artists@fnal.gov.";
  }

  //=============================================
  // Job level

  void
  EventProcessor::beginJob()
  {
    FDEBUG(1) << string(8, ' ') << "beginJob\n";
    breakpoints::beginJob();
    // NOTE: This implementation assumes 'Job' means one call the
    // EventProcessor::run. If it really means once per 'application'
    // then this code will have to be changed.  Also have to deal with
    // case where have 'run' then new Module added and do 'run' again.
    // In that case the newly added Module needs its 'beginJob' to be
    // called.
    try {
      input_.load()->doBeginJob();
    }
    catch (cet::exception& e) {
      mf::LogError("BeginJob") << "A cet::exception happened while processing"
                                  " the beginJob of the 'source'\n";
      e << "A cet::exception happened while processing"
           " the beginJob of the 'source'\n";
      throw;
    }
    catch (exception const&) {
      mf::LogError("BeginJob") << "A exception happened while processing"
                                  " the beginJob of the 'source'\n";
      throw;
    }
    catch (...) {
      mf::LogError("BeginJob") << "An unknown exception happened while"
                                  " processing the beginJob of the 'source'\n";
      throw;
    }
    (*schedule_.load())[ScheduleID::first()].beginJob();
    endPathExecutor_.load()->beginJob();
    actReg_.load()->sPostBeginJob.invoke();
    invokePostBeginJobWorkers_();
  }

  void
  EventProcessor::endJob()
  {
    FDEBUG(1) << string(8, ' ') << "endJob\n";
    ec_.load()->call(
      [this] { (*schedule_.load())[ScheduleID::first()].endJob(); });
    ec_.load()->call([this] { endPathExecutor_.load()->endJob(); });
    ec_.load()->call([] { ConsumesInfo::instance()->showMissingConsumes(); });
    ec_.load()->call([this] { input_.load()->doEndJob(); });
    ec_.load()->call([this] { actReg_.load()->sPostEndJob.invoke(); });
    ec_.load()->call([] { mf::LogStatistics(); });
    ec_.load()->call([this] {
      detail::writeSummary(*pathManager_.load(),
                           scheduler_.load()->wantSummary(),
                           *timer_.load());
    });
  }

  //====================================================
  // File level

  void
  EventProcessor::openInputFile()
  {
    actReg_.load()->sPreOpenFile.invoke();
    FDEBUG(1) << string(8, ' ') << "openInputFile\n";
    delete fb_.load();
    fb_ = nullptr;
    fb_ = input_.load()->readFile().release();
    if (fb_.load() == nullptr) {
      throw Exception(errors::LogicError)
        << "Source readFile() did not return a valid FileBlock: FileBlock "
        << "should be valid or readFile() should throw.\n";
    }
    actReg_.load()->sPostOpenFile.invoke(fb_.load()->fileName());
    respondToOpenInputFile();
  }

  void
  EventProcessor::closeAllFiles()
  {
    closeAllOutputFiles();
    closeInputFile();
  }

  void
  EventProcessor::closeInputFile()
  {
    endPathExecutor_.load()->incrementInputFileNumber();
    // Output-file closing on input-file boundaries are tricky since
    // input files must outlive the output files, which often have data
    // copied forward from the input files.  That's why the
    // recordOutputClosureRequests call is made here instead of in a
    // specialization of recordOutputModuleClosureRequests<>.
    endPathExecutor_.load()->recordOutputClosureRequests(
      Granularity::InputFile);
    if (endPathExecutor_.load()->outputsToClose()) {
      closeSomeOutputFiles();
    }
    respondToCloseInputFile();
    actReg_.load()->sPreCloseFile.invoke();
    input_.load()->closeFile();
    actReg_.load()->sPostCloseFile.invoke();
    FDEBUG(1) << string(8, ' ') << "closeInputFile\n";
  }

  void
  EventProcessor::openAllOutputFiles()
  {
    endPathExecutor_.load()->openAllOutputFiles(*fb_.load());
    FDEBUG(1) << string(8, ' ') << "openAllOutputFiles\n";
  }

  void
  EventProcessor::closeAllOutputFiles()
  {
    if (!endPathExecutor_.load()->someOutputsOpen()) {
      return;
    }
    respondToCloseOutputFiles();
    endPathExecutor_.load()->closeAllOutputFiles();
    FDEBUG(1) << string(8, ' ') << "closeAllOutputFiles\n";
  }

  void
  EventProcessor::openSomeOutputFiles()
  {
    if (!endPathExecutor_.load()->outputsToOpen()) {
      return;
    }
    endPathExecutor_.load()->openSomeOutputFiles(*fb_.load());
    FDEBUG(1) << string(8, ' ') << "openSomeOutputFiles\n";
    respondToOpenOutputFiles();
  }

  void
  EventProcessor::setOutputFileStatus(OutputFileStatus const ofs)
  {
    endPathExecutor_.load()->setOutputFileStatus(ofs);
    FDEBUG(1) << string(8, ' ') << "setOutputFileStatus\n";
  }

  void
  EventProcessor::closeSomeOutputFiles()
  {
    // Precondition: there are SOME output files that have been
    //               flagged as needing to close.  Otherwise,
    //               'respondtoCloseOutputFiles' will be needlessly
    //               called.
    assert(endPathExecutor_.load()->outputsToClose());
    respondToCloseOutputFiles();
    endPathExecutor_.load()->closeSomeOutputFiles();
    FDEBUG(1) << string(8, ' ') << "closeSomeOutputFiles\n";
  }

  void
  EventProcessor::respondToOpenInputFile()
  {
    (*schedule_.load())[ScheduleID::first()].respondToOpenInputFile(
      *fb_.load());
    endPathExecutor_.load()->respondToOpenInputFile(*fb_.load());
    FDEBUG(1) << string(8, ' ') << "respondToOpenInputFile\n";
  }

  void
  EventProcessor::respondToCloseInputFile()
  {
    (*schedule_.load())[ScheduleID::first()].respondToCloseInputFile(
      *fb_.load());
    endPathExecutor_.load()->respondToCloseInputFile(*fb_.load());
    FDEBUG(1) << string(8, ' ') << "respondToCloseInputFile\n";
  }

  void
  EventProcessor::respondToOpenOutputFiles()
  {
    (*schedule_.load())[ScheduleID::first()].respondToOpenOutputFiles(
      *fb_.load());
    endPathExecutor_.load()->respondToOpenOutputFiles(*fb_.load());
    FDEBUG(1) << string(8, ' ') << "respondToOpenOutputFiles\n";
  }

  void
  EventProcessor::respondToCloseOutputFiles()
  {
    (*schedule_.load())[ScheduleID::first()].respondToCloseOutputFiles(
      *fb_.load());
    endPathExecutor_.load()->respondToCloseOutputFiles(*fb_.load());
    FDEBUG(1) << string(8, ' ') << "respondToCloseOutputFiles\n";
  }

  //=============================================
  // Run level

  void
  EventProcessor::readRun()
  {
    actReg_.load()->sPreSourceRun.invoke();
    delete runPrincipal_.load();
    runPrincipal_ = nullptr;
    runPrincipal_ = input_.load()->readRun().release();
    assert(runPrincipal_.load());
    endPathExecutor_.load()->seedRunRangeSet(
      input_.load()->runRangeSetHandler());
    // The intended behavior here is that the producing services which are
    // called during the sPostReadRun cannot see each others put products. We
    // enforce this by creating the groups for the produced products, but do
    // not allow the lookups to find them until after the callbacks have run.
    runPrincipal_.load()->createGroupsForProducedProducts(
      *producedProductLookupTables_.load());
    psSignals_.load()->sPostReadRun.invoke(*runPrincipal_);
    runPrincipal_.load()->enableLookupOfProducedProducts(
      *producedProductLookupTables_.load());
    {
      Run const r{*runPrincipal_.load(), ModuleDescription{}};
      actReg_.load()->sPostSourceRun.invoke(r);
    }
    FDEBUG(1) << string(8, ' ') << "readRun.....................("
              << runPrincipal_.load()->runID() << ")\n";
  }

  void
  EventProcessor::beginRun()
  {
    assert(runPrincipal_.load());
    RunID const r{runPrincipal_.load()->runID()};
    if (r.isFlush()) {
      return;
    }
    finalizeRunEnabled_ = true;
    try {
      {
        Run const run{*runPrincipal_.load(), ModuleDescription{}};
        actReg_.load()->sPreBeginRun.invoke(run);
      }
      (*schedule_.load())[ScheduleID::first()].process(Transition::BeginRun,
                                                       *runPrincipal_.load());
      endPathExecutor_.load()->process(Transition::BeginRun,
                                       *runPrincipal_.load());
      {
        Run const run{*runPrincipal_.load(), ModuleDescription{}};
        actReg_.load()->sPostBeginRun.invoke(run);
      }
    }
    catch (cet::exception& ex) {
      throw Exception{
        errors::EventProcessorFailure,
        "EventProcessor: an exception occurred during current event processing",
        ex};
    }
    catch (...) {
      mf::LogError("PassingThrough")
        << "an exception occurred during current event processing\n";
      throw;
    }
    FDEBUG(1) << string(8, ' ') << "beginRun....................(" << r
              << ")\n";
    beginRunCalled_ = true;
  }

  void
  EventProcessor::beginRunIfNotDoneAlready()
  {
    if (!beginRunCalled_) {
      beginRun();
    }
  }

  void
  EventProcessor::setRunAuxiliaryRangeSetID()
  {
    assert(runPrincipal_.load());
    endPathExecutor_.load()->setAuxiliaryRangeSetID(*runPrincipal_.load());
    FDEBUG(1) << string(8, ' ') << "setRunAuxiliaryRangeSetID...("
              << runPrincipal_.load()->runID() << ")\n";
  }

  void
  EventProcessor::endRun()
  {
    assert(runPrincipal_.load());
    // Precondition: The RunID does not correspond to a flush ID. --
    // N.B. The flush flag is not explicitly checked here since endRun
    // is only called from finalizeRun, which is where the check
    // happens.
    RunID const run{runPrincipal_.load()->runID()};
    assert(!run.isFlush());
    try {
      {
        actReg_.load()->sPreEndRun.invoke(runPrincipal_.load()->runID(),
                                          runPrincipal_.load()->endTime());
      }
      (*schedule_.load())[ScheduleID::first()].process(Transition::EndRun,
                                                       *runPrincipal_.load());
      endPathExecutor_.load()->process(Transition::EndRun,
                                       *runPrincipal_.load());
      {
        Run const r{*runPrincipal_.load(), ModuleDescription{}};
        actReg_.load()->sPostEndRun.invoke(r);
      }
    }
    catch (cet::exception& ex) {
      throw Exception{
        errors::EventProcessorFailure,
        "EventProcessor: an exception occurred during current event processing",
        ex};
    }
    catch (...) {
      mf::LogError("PassingThrough")
        << "an exception occurred during current event processing\n";
      throw;
    }
    FDEBUG(1) << string(8, ' ') << "endRun......................(" << run
              << ")\n";
    beginRunCalled_ = false;
  }

  void
  EventProcessor::writeRun()
  {
    assert(runPrincipal_.load());
    // Precondition: The RunID does not correspond to a flush ID.
    RunID const r{runPrincipal_.load()->runID()};
    assert(!r.isFlush());
    endPathExecutor_.load()->writeRun(*runPrincipal_.load());
    FDEBUG(1) << string(8, ' ') << "writeRun....................(" << r
              << ")\n";
  }

  //=============================================
  // SubRun level

  void
  EventProcessor::readSubRun()
  {
    actReg_.load()->sPreSourceSubRun.invoke();
    delete subRunPrincipal_.load();
    subRunPrincipal_ = nullptr;
    subRunPrincipal_ =
      input_.load()->readSubRun(runPrincipal_.load()).release();
    assert(subRunPrincipal_.load());
    endPathExecutor_.load()->seedSubRunRangeSet(
      input_.load()->subRunRangeSetHandler());
    // The intended behavior here is that the producing services which are
    // called during the sPostReadSubRun cannot see each others put products. We
    // enforce this by creating the groups for the produced products, but do
    // not allow the lookups to find them until after the callbacks have run.
    subRunPrincipal_.load()->createGroupsForProducedProducts(
      *producedProductLookupTables_.load());
    psSignals_.load()->sPostReadSubRun.invoke(*subRunPrincipal_);
    subRunPrincipal_.load()->enableLookupOfProducedProducts(
      *producedProductLookupTables_.load());
    {
      SubRun const sr{*subRunPrincipal_.load(), ModuleDescription{}};
      actReg_.load()->sPostSourceSubRun.invoke(sr);
    }
    FDEBUG(1) << string(8, ' ') << "readSubRun..................("
              << subRunPrincipal_.load()->subRunID() << ")\n";
  }

  void
  EventProcessor::beginSubRun()
  {
    assert(subRunPrincipal_.load());
    SubRunID const sr{subRunPrincipal_.load()->subRunID()};
    if (sr.isFlush()) {
      return;
    }
    finalizeSubRunEnabled_ = true;
    try {
      {
        SubRun const srun{*subRunPrincipal_.load(), ModuleDescription{}};
        actReg_.load()->sPreBeginSubRun.invoke(srun);
      }
      (*schedule_.load())[ScheduleID::first()].process(
        Transition::BeginSubRun, *subRunPrincipal_.load());
      endPathExecutor_.load()->process(Transition::BeginSubRun,
                                       *subRunPrincipal_.load());
      {
        SubRun const srun{*subRunPrincipal_.load(), ModuleDescription{}};
        actReg_.load()->sPostBeginSubRun.invoke(srun);
      }
    }
    catch (cet::exception& ex) {
      throw Exception{
        errors::EventProcessorFailure,
        "EventProcessor: an exception occurred during current event processing",
        ex};
    }
    catch (...) {
      mf::LogError("PassingThrough")
        << "an exception occurred during current event processing\n";
      throw;
    }
    FDEBUG(1) << string(8, ' ') << "beginSubRun.................(" << sr
              << ")\n";
    beginSubRunCalled_ = true;
  }

  void
  EventProcessor::beginSubRunIfNotDoneAlready()
  {
    if (!beginSubRunCalled_) {
      beginSubRun();
    }
  }

  void
  EventProcessor::setSubRunAuxiliaryRangeSetID()
  {
    assert(subRunPrincipal_.load());
    endPathExecutor_.load()->setAuxiliaryRangeSetID(*subRunPrincipal_.load());
    FDEBUG(1) << string(8, ' ') << "setSubRunAuxiliaryRangeSetID("
              << subRunPrincipal_.load()->subRunID() << ")\n";
  }

  void
  EventProcessor::endSubRun()
  {
    assert(subRunPrincipal_.load());
    // Precondition: The SubRunID does not correspond to a flush ID.
    // Note: the flush flag is not explicitly checked here since
    // endSubRun is only called from finalizeSubRun, which is where the
    // check happens.
    SubRunID const sr{subRunPrincipal_.load()->subRunID()};
    assert(!sr.isFlush());
    try {
      {
        actReg_.load()->sPreEndSubRun.invoke(
          subRunPrincipal_.load()->subRunID(),
          subRunPrincipal_.load()->endTime());
      }
      (*schedule_.load())[ScheduleID::first()].process(
        Transition::EndSubRun, *subRunPrincipal_.load());
      endPathExecutor_.load()->process(Transition::EndSubRun,
                                       *subRunPrincipal_.load());
      {
        SubRun const srun{*subRunPrincipal_.load(), ModuleDescription{}};
        actReg_.load()->sPostEndSubRun.invoke(srun);
      }
    }
    catch (cet::exception& ex) {
      throw Exception{
        errors::EventProcessorFailure,
        "EventProcessor: an exception occurred during current event processing",
        ex};
    }
    catch (...) {
      mf::LogError("PassingThrough")
        << "an exception occurred during current event processing\n";
      throw;
    }
    FDEBUG(1) << string(8, ' ') << "endSubRun...................(" << sr
              << ")\n";
    beginSubRunCalled_ = false;
  }

  void
  EventProcessor::writeSubRun()
  {
    assert(subRunPrincipal_.load());
    // Precondition: The SubRunID does not correspond to a flush ID.
    SubRunID const& sr{subRunPrincipal_.load()->subRunID()};
    assert(!sr.isFlush());
    endPathExecutor_.load()->writeSubRun(*subRunPrincipal_.load());
    FDEBUG(1) << string(8, ' ') << "writeSubRun.................(" << sr
              << ")\n";
  }

  void
  EventProcessor::terminateAbnormally_() try {
    if (ServiceRegistry::isAvailable<RandomNumberGenerator>()) {
      ServiceHandle<RandomNumberGenerator> {}
      ->saveToFile_();
    }
  }
  catch (...) {
  }

} // namespace art
