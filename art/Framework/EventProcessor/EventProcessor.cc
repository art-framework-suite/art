#include "art/Framework/EventProcessor/EventProcessor.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Breakpoints.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceFactory.h"
#include "art/Framework/Core/InputSourceMutex.h"
#include "art/Framework/EventProcessor/detail/writeSummary.h"
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
#include "art/Utilities/Globals.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/SharedResourcesRegistry.h"
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
#include "hep_concurrency/tsan.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "tbb/task.h"
#include "tbb/task_arena.h"

#include <algorithm>
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
    auto it = eventPrincipals_->cbegin();
    auto const e = eventPrincipals_->cend();
    for (; it != e; ++it) {
      delete *it;
    }
    ParentageRegistry::instance(true);
    ProcessConfigurationRegistry::instance(true);
    ProcessHistoryRegistry::instance(true);
    TypeID::shutdown();
    ANNOTATE_THREAD_IGNORE_END;
  }

  namespace {
    ServicesManager*
    create_services_manager(ParameterSet services_pset,
                            ActivityRegistry& actReg)
    {
      auto const fpcPSet =
        services_pset.get<ParameterSet>("FloatingPointControl", {});
      services_pset.erase("FloatingPointControl");
      services_pset.erase("message");
      services_pset.erase("scheduler");
      auto mgr = new ServicesManager{move(services_pset), actReg};
      mgr->addSystemService<FloatingPointControl>(fpcPSet, actReg);
      return mgr;
    }

    auto const invalid_module_context = ModuleContext::invalid();
  }

  EventProcessor::EventProcessor(ParameterSet const& pset)
    : scheduler_{pset.get<ParameterSet>("services.scheduler")}
    , scheduleIteration_{static_cast<ScheduleID::size_type>(
        scheduler_->num_schedules())}
    , servicesManager_{create_services_manager(
        pset.get<ParameterSet>("services"),
        actReg_)}
    , pathManager_{pset,
                   outputCallbacks_,
                   producedProductDescriptions_,
                   scheduler_->actionTable(),
                   actReg_}
    , handleEmptyRuns_{scheduler_->handleEmptyRuns()}
    , handleEmptySubRuns_{scheduler_->handleEmptySubRuns()}
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
    }
    ParentageRegistry::instance();
    ProcessConfigurationRegistry::instance();
    ProcessHistoryRegistry::instance();
    SharedResourcesRegistry::instance();
    ServiceRegistry::instance().setManager(servicesManager_.get());
    // We do this late because the floating point control word, signal
    // masks, etc., are per-thread and inherited from the master
    // thread, so we want to allow system services, user services, and
    // modules to configure these things in their constructors before
    // we let tbb create any threads. This means they cannot use tbb
    // in their constructors, instead they must use the beginJob
    // callout.
    scheduler_->initialize_task_manager();
    // Whenever we are ready to enable ROOT's implicit MT, which is
    // equivalent to its use of TBB, the call should be made after our
    // own TBB task manager has been initialized.
    //    ROOT::EnableImplicitMT();
    auto const& processName{pset.get<string>("process_name")};
    Globals::instance()->setProcessName(processName);
    {
      ostringstream msg;
      msg << "nschedules: " << scheduler_->num_schedules()
          << " nthreads: " << scheduler_->num_threads();
      TDEBUG_FUNC_MSG(5, "EventProcessor::EventProcessor", msg.str());
    }
    ParameterSet triggerPSet;
    triggerPSet.put("trigger_paths", pathManager_->triggerPathNames());
    fhicl::ParameterSetRegistry::put(triggerPSet);
    Globals::instance()->setTriggerPSet(triggerPSet);
    Globals::instance()->setTriggerPathNames(pathManager_->triggerPathNames());
    eventPrincipals_->expand_to_num_schedules();
    auto annotate_principal = [this](ScheduleID const sid) {
      auto ep [[maybe_unused]] = &eventPrincipals_->at(sid);
      ANNOTATE_BENIGN_RACE_SIZED(
        ep, sizeof(EventPrincipal*), "EventPrincipal ptr");
    };
    scheduleIteration_.for_each_schedule(annotate_principal);
    auto const errorOnMissingConsumes = scheduler_->errorOnMissingConsumes();
    ConsumesInfo::instance()->setRequireConsumes(errorOnMissingConsumes);
    {
      auto const& physicsPSet = pset.get<ParameterSet>("physics", {});
      servicesManager_->addSystemService<TriggerNamesService>(
        pathManager_->triggerPathNames(),
        processName,
        triggerPSet,
        physicsPSet);
    }
    // We have delayed creating the service instances, now actually
    // create them.
    servicesManager_->forceCreation();
    ServiceHandle<FileCatalogMetadata> {}
    ->addMetadataString("art.process_name", processName);

    // Now that the service module instances have been created we can
    // set the callbacks, set the module description, and register the
    // products for each service module instance.
    ProcessConfiguration const pc{processName, pset.id(), getReleaseVersion()};
    auto const producing_services = servicesManager_->registerProducts(
      producedProductDescriptions_, psSignals_, pc);
    pathManager_->createModulesAndWorkers(producing_services);
    auto create =
      [&processName, &pset, &triggerPSet, this](ScheduleID const sid) {
        endPathExecutors_->emplace(
          std::piecewise_construct,
          std::forward_as_tuple(sid),
          std::forward_as_tuple(sid,
                                pathManager_,
                                scheduler_->actionTable(),
                                actReg_,
                                outputCallbacks_));
        schedules_->emplace(std::piecewise_construct,
                            std::forward_as_tuple(sid),
                            std::forward_as_tuple(sid,
                                                  pathManager_,
                                                  processName,
                                                  pset,
                                                  triggerPSet,
                                                  outputCallbacks_,
                                                  producedProductDescriptions_,
                                                  scheduler_->actionTable(),
                                                  actReg_));
      };
    scheduleIteration_.for_each_schedule(create);
    SharedResourcesRegistry::instance()->freeze();

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
        ModuleThreadingType::legacy,
        ProcessConfiguration{processName, pset.id(), getReleaseVersion()}};
      InputSourceDescription isd{md, outputCallbacks_, actReg_};
      try {
        input_.reset(InputSourceFactory::make(main_input, isd).release());
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
    actReg_->sPostSourceConstruction.invoke(input_->moduleDescription());
    // Create product tables used for product retrieval within modules.
    producedProductLookupTables_ = ProductTables{producedProductDescriptions_};
    outputCallbacks_->invoke(producedProductLookupTables_);
  }

  void
  EventProcessor::invokePostBeginJobWorkers_()
  {
    using cet::transform_all;
    // Need to convert multiple lists of workers into a long list that
    // the postBeginJobWorkers callbacks can understand.
    vector<Worker*> allWorkers;
    transform_all(pathManager_->triggerPathsInfo(ScheduleID::first()).workers(),
                  back_inserter(allWorkers),
                  [](auto const& pr) { return pr.second; });
    transform_all(pathManager_->endPathInfo(ScheduleID::first()).workers(),
                  back_inserter(allWorkers),
                  [](auto const& pr) { return pr.second; });
    actReg_->sPostBeginJobWorkers.invoke(input_, allWorkers);
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
      if (endPathExecutors_->at(ScheduleID::first()).outputsToClose()) {
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
    throw Exception{errors::LogicError} << "Incorrect level hierarchy.\n"
                                        << "  Current level: " << L
                                        << "  Next level: " << nextLevel_;
  }

  // Specializations for process function template

  template <>
  inline void
  EventProcessor::begin<Level::Job>()
  {
    timer_->start();
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
    if (handleEmptyRuns_) {
      beginRun();
    }
  }

  template <>
  void
  EventProcessor::begin<Level::SubRun>()
  {
    finalizeSubRunEnabled_ = true;
    assert(runPrincipal_);
    assert(runPrincipal_->runID().isValid());
    readSubRun();
    if (handleEmptySubRuns_) {
      beginRunIfNotDoneAlready();
      beginSubRun();
    }
  }

  template <>
  void
  EventProcessor::finalize<Level::SubRun>()
  {
    assert(subRunPrincipal_);
    if (!finalizeSubRunEnabled_) {
      return;
    }
    if (subRunPrincipal_->subRunID().isFlush()) {
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
    assert(runPrincipal_);
    if (!finalizeRunEnabled_) {
      return;
    }
    if (runPrincipal_->runID().isFlush()) {
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
    timer_->stop();
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
    endPathExecutors_->at(ScheduleID::first())
      .recordOutputClosureRequests(Granularity::Run);
  }

  template <>
  void
  EventProcessor::recordOutputModuleClosureRequests<Level::SubRun>()
  {
    endPathExecutors_->at(ScheduleID::first())
      .recordOutputClosureRequests(Granularity::SubRun);
  }

  template <>
  void
  EventProcessor::recordOutputModuleClosureRequests<Level::Event>()
  {
    endPathExecutors_->at(ScheduleID::first())
      .recordOutputClosureRequests(Granularity::Event);
  }

  class ProcessAllEventsFunctor {
  public:
    ProcessAllEventsFunctor(EventProcessor* evp,
                            tbb::task* eventLoopTask,
                            ScheduleID const sid)
      : evp_(evp), eventLoopTask_(eventLoopTask), sid_(sid)
    {}
    void
    operator()(exception_ptr const* ex)
    {
      evp_->processAllEventsTask(eventLoopTask_, sid_, ex);
    }

  private:
    EventProcessor* evp_;
    tbb::task* eventLoopTask_;
    ScheduleID const sid_;
  };

  void
  EventProcessor::processAllEventsTask(tbb::task* eventLoopTask,
                                       ScheduleID const sid,
                                       exception_ptr const*)
  {
    TDEBUG_BEGIN_TASK_SI(4, "processAllEventsTask", sid);
    processAllEventsAsync(eventLoopTask, sid);
    TDEBUG_END_TASK_SI(4, "processAllEventsTask", sid);
  }

  template <>
  void
  EventProcessor::process<most_deeply_nested_level()>()
  {
    if ((shutdown_flag > 0) || !ec_->empty()) {
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
      eventLoopTask->set_ref_count(scheduler_->num_schedules() + 1);

      tbb::task_list schedule_heads;
      auto create_event_processing_tasks = [&eventLoopTask,
                                            &schedule_heads,
                                            this](ScheduleID const sid) {
        auto processAllEventsTask =
          make_waiting_task(eventLoopTask->allocate_child(),
                            ProcessAllEventsFunctor{this, eventLoopTask, sid});
        schedule_heads.push_back(*processAllEventsTask);
      };
      scheduleIteration_.for_each_schedule(create_event_processing_tasks);

      eventLoopTask->spawn_and_wait_for_all(schedule_heads);
      tbb::task::destroy(*eventLoopTask);
      // If anything bad happened during event processing, let the
      // user know.
      if (deferredExceptionPtrIsSet_.load()) {
        rethrow_exception(deferredExceptionPtr_);
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
        endPathExecutors_->at(ScheduleID::first()).closeSomeOutputFiles();
        FDEBUG(1) << string(8, ' ') << "closeSomeOutputFiles\n";
      }
      // We started the switch after advancing to the next item type;
      // we must make sure that we read that event before advancing
      // the item type again.
      firstEvent_ = true;
      fileSwitchInProgress_ = false;
    }
  }

  class ReadAndProcessEventFunctor {
  public:
    ReadAndProcessEventFunctor(EventProcessor* const evp,
                               tbb::task* const eventLoopTask,
                               ScheduleID const sid)
      : evp_{evp}, eventLoopTask_{eventLoopTask}, sid_{sid}
    {}
    void
    operator()(exception_ptr const* ex)
    {
      evp_->readAndProcessEventTask(eventLoopTask_, sid_, ex);
    }

  private:
    EventProcessor* const evp_;
    tbb::task* const eventLoopTask_;
    ScheduleID const sid_;
  };

  void
  EventProcessor::readAndProcessEventTask(tbb::task* eventLoopTask,
                                          ScheduleID const sid,
                                          exception_ptr const*)
  {
    // Note: When we come here our parent is the eventLoop task.
    TDEBUG_BEGIN_TASK_SI(4, "readAndProcessEventTask", sid);
    try {
      // Note: We pass eventLoopTask here to keep the thread sanitizer happy.
      readAndProcessAsync(eventLoopTask, sid);
    }
    catch (...) {
      // Use a thread-safe, one-way trapdoor pattern to notify the
      // main thread of the exception.
      bool expected{false};
      if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected, true)) {
        // Put the exception where the main thread can get at it.
        deferredExceptionPtr_ = current_exception();
      }
      // And then end this task, terminating event processing.
      TDEBUG_END_TASK_SI_ERR(4,
                             "readAndProcessEventTask",
                             sid,
                             "terminate event loop because of EXCEPTION");
      return;
    }
    // If no exception, then end this task, which does not terminate
    // event processing because our parent is the nullptr because we
    // transferred it to another task.
    TDEBUG_END_TASK_SI(4, "readAndProcessEventTask", sid);
  }

  // This is the event loop (also known as the schedule head).  It
  // makes a continuation task (the readAndProcessEventTask) which
  // reads and processes a single event, creates itself again as a
  // continuation task, and then exits. We are passed the
  // EventLoopTask here to keep the thread sanitizer happy.
  void
  EventProcessor::processAllEventsAsync(tbb::task* eventLoopTask,
                                        ScheduleID const sid)
  {
    // Note: We are part of the processAllEventsTask (schedule head task),
    // and our parent is the eventLoopTask.
    TDEBUG_BEGIN_FUNC_SI(4, "EventProcessor::processAllEventsAsync", sid);
    // Create a continuation task that has the EventLoopTask as parent,
    // and reset our parent to the nullptr at the same time, which means when
    // we end we do not decrement the ref count of the EventLoopTask and our
    // continuation runs, not the EventLoopTask.
    auto readAndProcessEventTask =
      make_waiting_task(tbb::task::self().allocate_continuation(),
                        ReadAndProcessEventFunctor{this, eventLoopTask, sid});
    // Push the readAndProcessEventTask onto the end of this thread's
    // TBB scheduling dequeue.
    tbb::task::spawn(*readAndProcessEventTask);
    // And end this task, which does not terminate event processing
    // because our parent is the nullptr because we transferred it
    // to the readAndProcessEventTask with allocate_continuation().
    TDEBUG_END_FUNC_SI(4, "EventProcessor::processAllEventsAsync", sid);
    // Note: We end and our continuation, the readAndProcessEventTask,
    // begins on this same thread because it is the first task on this
    // thread's TBB scheduling deque, and we have no parent, and we
    // return the nullptr.
  }

  // This function is executed as part of the readAndProcessEvent task,
  // our parent task is the EventLoopTask. Here we advance to the next item
  // in the file index, end event processing if it is not an event, or if
  // the user has requested a shutdown, read the event, and then call another
  // function to do the processing.
  void
  EventProcessor::readAndProcessAsync(tbb::task* EventLoopTask,
                                      ScheduleID const sid)
  {
    // Note: We are part of the readAndProcessEventTask (schedule head task),
    // and our parent task is the EventLoopTask.
    TDEBUG_BEGIN_FUNC_SI(4, "EventProcessor::readAndProcessAsync", sid);
    // Note: shutdown_flag is a extern global atomic int in
    // art/art/Utilities/UnixSignalHandlers.cc
    if (shutdown_flag) {
      // User called for a clean shutdown using a signal or ctrl-c,
      // end event processing and this task.
      TDEBUG_END_FUNC_SI_ERR(4, "readAndProcessAsync", sid, "CLEAN SHUTDOWN");
      return;
    }
    // The item type advance and the event read must be done with the
    // input source lock held; however event-processing must not
    // serialized.
    {
      InputSourceMutexSentry lock_input;
      auto do_switch = fileSwitchInProgress_.load();
      if (do_switch) {
        // We must avoid advancing the iterator after a schedule has
        // noticed it is time to switch files.  After the switch, we
        // will need to set firstEvent_ true so that the first
        // schedule that resumes after the switch actually reads the
        // event that the first schedule which noticed we needed a
        // switch had advanced the iterator to.

        // Note: We still have the problem that because the schedules
        // do not read events at the same time the file switch point
        // can be up to #nschedules-1 ahead of where it would have
        // been if there was only one schedule.  If we are switching
        // output files every event in an attempt to create single
        // event files, this really does not work out too well.
        TDEBUG_END_FUNC_SI_ERR(
          4, "EventProcessor::readAndProcessAsync", sid, "FILE SWITCH");
        return;
      }
      // Check the next item type and exit this task if it is not an
      // event, or if the user has asynchronously requested a
      // shutdown.
      auto expected = true;
      if (firstEvent_.compare_exchange_strong(expected, false)) {
        // Do not advance the item type on the first event.
      } else {
        // Do the advance item type.
        if (nextLevel_.load() == Level::ReadyToAdvance) {
          // See what the next item is.
          TDEBUG_FUNC_SI_MSG(5,
                             "EventProcessor::readAndProcessAsync",
                             sid,
                             "Calling advanceItemType()");
          nextLevel_ = advanceItemType();
        }
        if ((nextLevel_.load() < most_deeply_nested_level()) ||
            (nextLevel_.load() == highest_level())) {
          // We are popping up, end event processing and this task.
          TDEBUG_END_FUNC_SI_ERR(
            4, "EventProcessor::readAndProcessAsync", sid, "END OF SUBRUN");
          return;
        }
        if (nextLevel_.load() != most_deeply_nested_level()) {
          // Error: incorrect level hierarchy
          TDEBUG_END_FUNC_SI_ERR(
            4, "EventProcessor::readAndProcessAsync", sid, "BAD HIERARCHY");
          throw Exception{errors::LogicError} << "Incorrect level hierarchy.";
        }
        nextLevel_ = Level::ReadyToAdvance;
        // At this point we have determined that we are going to read an event
        // and we must do that before dropping the lock on the input source
        // which is what is protecting us against a double-advance caused by
        // a different schedule.
        auto mustClose = endPathExecutors_->at(sid).outputsToClose();
        if (mustClose) {
          fileSwitchInProgress_ = true;
          TDEBUG_END_FUNC_SI_ERR(4,
                                 "EventProcessor::readAndProcessAsync",
                                 sid,
                                 "FILE SWITCH INITIATED");
          return;
        }
      }
      //
      //  Now we can read the event from the source.
      //
      ScheduleContext const sc{sid};
      assert(subRunPrincipal_);
      // FIXME: This assert is causing a data race!
      // assert(subRunPrincipal_->subRunID().isValid());
      {
        actReg_->sPreSourceEvent.invoke(sc);
      }
      TDEBUG_FUNC_SI_MSG(5,
                         "readAndProcessAsync",
                         sid,
                         "Calling input_->readEvent(subRunPrincipal_)");
      eventPrincipals_->at(sid) =
        input_->readEvent(subRunPrincipal_.get()).release();
      assert(eventPrincipals_->at(sid));
      // The intended behavior here is that the producing services
      // which are called during the sPostReadEvent cannot see each
      // others put products.  We enforce this by creating the groups
      // for the produced products, but do not allow the lookups to
      // find them until after the callbacks have run.
      eventPrincipals_->at(sid)->createGroupsForProducedProducts(
        producedProductLookupTables_);
      psSignals_->sPostReadEvent.invoke(*eventPrincipals_->at(sid));
      eventPrincipals_->at(sid)->enableLookupOfProducedProducts(
        producedProductLookupTables_);
      {
        Event const e{*eventPrincipals_->at(sid), invalid_module_context};
        actReg_->sPostSourceEvent.invoke(e, sc);
      }
      FDEBUG(1) << string(8, ' ') << "readEvent...................("
                << eventPrincipals_->at(sid)->eventID() << ")\n";
      // Now we drop the input source lock by exiting the guarded scope.
    }
    if (eventPrincipals_->at(sid)->eventID().isFlush()) {
      // No processing to do, start next event handling task,
      // transferring our parent task (EventLoopTask) to it,
      // and exit this task.
      processAllEventsAsync(EventLoopTask, sid);
      TDEBUG_END_FUNC_SI_ERR(
        4, "EventProcessor::readAndProcessAsync", sid, "FLUSH EVENT");
      return;
    }
    // Now process the event.
    processEventAsync(EventLoopTask, sid);
    // And end this task, which does not terminate event processing
    // because our parent is the nullptr because we transferred it to
    // the endPathTask.
    TDEBUG_END_FUNC_SI(4, "EventProcessor::readAndProcessAsync", sid);
  }

  class EndPathFunctor {
  public:
    EndPathFunctor(EventProcessor* evp,
                   tbb::task* eventLoopTask,
                   ScheduleID const sid)
      : evp_{evp}, eventLoopTask_{eventLoopTask}, sid_{sid}
    {}
    void
    operator()(exception_ptr const* ex)
    {
      evp_->endPathTask(eventLoopTask_, sid_, ex);
    }

  private:
    EventProcessor* evp_;
    tbb::task* eventLoopTask_;
    ScheduleID const sid_;
  };

  void
  EventProcessor::endPathTask(tbb::task* eventLoopTask,
                              ScheduleID const sid,
                              exception_ptr const* ex)
  {
    // Note: When we start our parent is the eventLoopTask.
    TDEBUG_BEGIN_TASK_SI(4, "endPathTask", sid);
    if (ex != nullptr) {
      try {
        rethrow_exception(*ex);
      }
      catch (cet::exception& e) {
        if (scheduler_->actionTable().find(e.root_cause()) !=
            actions::IgnoreCompletely) {
          // Use a thread-safe, one-way trapdoor pattern to notify the
          // main thread of the exception.
          bool expected = false;
          if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected,
                                                                 true)) {
            // Put the exception where the main thread can get at it.
            deferredExceptionPtr_ = make_exception_ptr(
              Exception{errors::EventProcessorFailure,
                        "EventProcessor: an exception occurred during current "
                        "event processing",
                        e});
          }
          // And then end this task, terminating event processing.
          TDEBUG_END_TASK_SI_ERR(
            4, "endPathTask", sid, "terminate event loop because of EXCEPTION");
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
          deferredExceptionPtr_ = current_exception();
        }
        // And then end this task, terminating event processing.
        TDEBUG_END_TASK_SI_ERR(
          4, "endPathTask", sid, "terminate event loop because of EXCEPTION");
        return;
      }
      // WARNING: We only get here if the trigger paths threw and we
      // are ignoring the exception because of
      // actions::IgnoreCompletely.
    }
    processEndPathAsync(eventLoopTask, sid);
    // And end this task, which does not terminate event processing
    // because our parent is the nullptr because it got transferred to
    // the next process event task.
    TDEBUG_END_TASK_SI(4, "endPathTask", sid);
  }

  // This function is a continuation of the body of the
  // readAndProcessEvent task. Here we call down to Schedule to do the
  // trigger path processing, passing it a waiting task which will do
  // the end path processing, finalize the event, and start the next
  // read and process event task.  Note that Schedule will spawn a
  // task to process each of the trigger paths, and then when they are
  // finished, insert the trigger results, and then spawn the waiting
  // task we gave it to do the end path processing, write the event,
  // and then start the next event processing task.
  void
  EventProcessor::processEventAsync(tbb::task* eventLoopTask,
                                    ScheduleID const sid)
  {
    // Note: We are part of the readAndProcessEventTask (schedule head
    // task), and our parent task is the EventLoopTask.
    TDEBUG_BEGIN_FUNC_SI(4, "EventProcessor::processEventAsync", sid);
    assert(eventPrincipals_->at(sid));
    assert(!eventPrincipals_->at(sid)->eventID().isFlush());
    try {
      // Make the end path processing task, make its parent
      // the EventLoopTask, and set our parent to the nullptr
      // so we can exit without ending event processing.
      auto endPathTask =
        make_waiting_task(tbb::task::self().allocate_continuation(),
                          EndPathFunctor{this, eventLoopTask, sid});
      {
        ScheduleContext const sc{sid};
        Event const ev{*eventPrincipals_->at(sid), invalid_module_context};
        actReg_->sPreProcessEvent.invoke(ev, sc);
      }
      // Start the trigger paths running.  When they finish
      // they will spawn the endPathTask which will run the
      // end path, write the event, and start the next event
      // processing task.
      schedules_->at(sid).process_event(
        endPathTask, eventLoopTask, *eventPrincipals_->at(sid));
      // Once the trigger paths are running we are done, exit this task,
      // which does not end event processing because our parent is the
      // nullptr because we transferred it to the endPathTask above.
      TDEBUG_END_FUNC_SI(4, "EventProcessor::processEventAsync", sid);
      return;
    }
    catch (cet::exception& e) {
      if (scheduler_->actionTable().find(e.root_cause()) !=
          actions::IgnoreCompletely) {
        // Use a thread-safe, one-way trapdoor pattern to notify the main
        // thread of the exception.
        bool expected = false;
        if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected,
                                                               true)) {
          // Put the exception where the main thread can get at it.
          deferredExceptionPtr_ =
            make_exception_ptr(Exception{errors::EventProcessorFailure,
                                         "EventProcessor: an exception "
                                         "occurred during current event "
                                         "processing",
                                         e});
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
                               sid,
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
        deferredExceptionPtr_ = current_exception();
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
                             sid,
                             "terminate event loop "
                             "because of EXCEPTION");
      return;
    }
    // WARNING: The only way to get here is if starting the trigger path
    // processing threw and actions::IgnoreCompletely is set!
    finishEventAsync(eventLoopTask, sid);
    // And then end this task, which does not end event
    // processing because we finishEvent transferred our
    // parent to the next event processing task.
    TDEBUG_END_FUNC_SI(4, "EventProcessor::processEventAsync", sid);
  }

  class EndPathRunnerFunctor {
  public:
    EndPathRunnerFunctor(EventProcessor* evp,
                         ScheduleID const sid,
                         tbb::task* eventLoopTask)
      : evp_{evp}, sid_{sid}, eventLoopTask_{eventLoopTask}
    {}
    void
    operator()() const
    {
      evp_->endPathRunnerTask(sid_, eventLoopTask_);
    }

  private:
    EventProcessor* evp_;
    ScheduleID const sid_;
    tbb::task* eventLoopTask_;
  };

  void
  EventProcessor::endPathRunnerTask(ScheduleID const sid,
                                    tbb::task* eventLoopTask)
  {
    TDEBUG_BEGIN_TASK_SI(4, "endPathFunctor", sid);
    // Arrange it so that we can terminate event
    // processing if we want to.
    ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                 sizeof(tbb::internal::task_prefix),
                               sizeof(tbb::task) +
                                 sizeof(tbb::internal::task_prefix),
                               "tbb::task");
    tbb::task::self().set_parent(eventLoopTask);
    try {
      endPathExecutors_->at(sid).process_event(*eventPrincipals_->at(sid));
    }
    catch (cet::exception& e) {
      // Possible actions: IgnoreCompletely, Rethrow, SkipEvent, FailModule,
      // FailPath
      if (scheduler_->actionTable().find(e.root_cause()) !=
          actions::IgnoreCompletely) {
        // Possible actions: Rethrow, SkipEvent, FailModule, FailPath
        // Use a thread-safe, one-way trapdoor pattern to
        // notify the main thread of the exception.
        bool expected = false;
        if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected,
                                                               true)) {
          // Put the exception where the main thread can get at it.
          deferredExceptionPtr_ = make_exception_ptr(
            Exception{errors::EventProcessorFailure,
                      "EventProcessor: an exception occurred during "
                      "current event processing",
                      e});
        }
        // And then end this task, terminating event processing.
        ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                     sizeof(tbb::internal::task_prefix),
                                   sizeof(tbb::task) +
                                     sizeof(tbb::internal::task_prefix),
                                   "tbb::task");
        tbb::task::self().set_parent(eventLoopTask);
        TDEBUG_END_TASK_SI_ERR(4,
                               "endPathFunctor",
                               sid,
                               "terminate event loop because of EXCEPTION");
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
      // Use a thread-safe, one-way trapdoor pattern to notify the
      // main thread of the exception.
      bool expected = false;
      if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected, true)) {
        // Put the exception where the main thread can get at it.
        deferredExceptionPtr_ = current_exception();
      }
      // And then end this task, terminating event processing.
      ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                   sizeof(tbb::internal::task_prefix),
                                 sizeof(tbb::task) +
                                   sizeof(tbb::internal::task_prefix),
                                 "tbb::task");
      tbb::task::self().set_parent(eventLoopTask);
      TDEBUG_END_TASK_SI_ERR(
        4, "endPathFunctor", sid, "terminate event loop because of EXCEPTION");
      return;
    }
    {
      ScheduleContext const sc{sid};
      Event const ev{*eventPrincipals_->at(sid), invalid_module_context};
      actReg_->sPostProcessEvent.invoke(ev, sc);
    }
    finishEventAsync(eventLoopTask, sid);
    // Note that we do not terminate event processing when we end
    // because finishEventAsync has set our parent to the nullptr.
    TDEBUG_END_TASK_SI(4, "endPathFunctor", sid);
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
  // the n schedules end when we push onto the queue, and are
  // recreated after the event is written to process the next event.
  void
  EventProcessor::processEndPathAsync(tbb::task* eventLoopTask,
                                      ScheduleID const sid)
  {
    // Note: We are part of the endPathTask.
    TDEBUG_BEGIN_FUNC_SI(4, "EventProcessor::processEndPathAsync", sid);
    // Arrange it so that we can end the task without terminating
    // event processing.
    ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                 sizeof(tbb::internal::task_prefix),
                               sizeof(tbb::task) +
                                 sizeof(tbb::internal::task_prefix),
                               "tbb::task");
    tbb::task::self().set_parent(nullptr);
    endPathQueue_->push(EndPathRunnerFunctor{this, sid, eventLoopTask});
    // Once the end path processing and event finalization processing
    // is queued we are done, exit this task, which does not end event
    // processing because our parent is the nullptr because we transferred
    // it to the endPathFunctor.
    TDEBUG_END_FUNC_SI(4, "EventProcessor::processEndPathAsync", sid);
  }

  // This function is a continuation of the Process End Path task, or
  // the error handling for an ignored trigger path exception in the
  // Process All Events task, or an ignored end path exception in the
  // Process End Path task.  Our parent task is the eventLoopTask.
  // We write the event out, spawn the next event processing task, and
  // end the current task.
  void
  EventProcessor::finishEventAsync(tbb::task* eventLoopTask,
                                   ScheduleID const sid)
  {
    // Note: We are part of the endPathFunctor.
    TDEBUG_BEGIN_FUNC_SI(4, "EventProcessor::finishEventAsync", sid);
    FDEBUG(1) << string(8, ' ') << "processEvent................("
              << eventPrincipals_->at(sid)->eventID() << ")\n";
    try {
      // Ask the output workers if they have reached their limits,
      // and if so setup to end the job the next time around the
      // event loop.
      FDEBUG(1) << string(8, ' ') << "shouldWeStop\n";
      TDEBUG_FUNC_SI_MSG(5,
                         "EventProcessor::finishEventAsync",
                         sid,
                         "Calling endPathExecutors_->allAtLimit()");
      if (endPathExecutors_->at(sid).allAtLimit()) {
        // Set to return to the File level.
        nextLevel_ = highest_level();
      }
      // Now we can write the results of processing to the outputs,
      // and delete the event principal.
      assert(eventPrincipals_->at(sid));
      auto isFlush = eventPrincipals_->at(sid)->eventID().isFlush();
      if (!isFlush) {
        // Possibly open new output files.  This is safe to do because
        // EndPathExecutor functions are called in a serialized
        // context.
        TDEBUG_FUNC_SI_MSG(5,
                           "EventProcessor::finishEventAsync",
                           sid,
                           "Calling openSomeOutputFiles()");
        openSomeOutputFiles();
        assert(eventPrincipals_->at(sid));
        assert(!eventPrincipals_->at(sid)->eventID().isFlush());
        TDEBUG_FUNC_SI_MSG(5,
                           "EventProcessor::finishEventAsync",
                           sid,
                           "Calling endPathExecutors_->writeEvent(sid, "
                           "*eventPrincipals_->at(sid))");
        // Write the event.
        endPathExecutors_->at(sid).writeEvent(*eventPrincipals_->at(sid));
        FDEBUG(1) << string(8, ' ') << "writeEvent..................("
                  << eventPrincipals_->at(sid)->eventID() << ")\n";
        // And delete the event principal.
        delete eventPrincipals_->at(sid);
        eventPrincipals_->at(sid) = nullptr;
      }
      TDEBUG_FUNC_SI_MSG(5,
                         "EventProcessor::finishEventAsync",
                         sid,
                         "Calling endPathExecutors_->"
                         "recordOutputClosureRequests(Granularity::Event)");
      endPathExecutors_->at(sid).recordOutputClosureRequests(
        Granularity::Event);
    }
    catch (cet::exception& e) {
      if (scheduler_->actionTable().find(e.root_cause()) !=
          actions::IgnoreCompletely) {
        // Use a thread-safe, one-way trapdoor pattern to notify the
        // main thread of the exception.
        bool expected = false;
        if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected,
                                                               true)) {
          // Put the exception where the main thread can get at it.
          deferredExceptionPtr_ = make_exception_ptr(
            Exception{errors::EventProcessorFailure,
                      "EventProcessor: an exception occurred during "
                      "current event processing",
                      e});
        }
        // And then end this task, terminating event processing.
        ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                     sizeof(tbb::internal::task_prefix),
                                   sizeof(tbb::task) +
                                     sizeof(tbb::internal::task_prefix),
                                   "tbb::task");
        tbb::task::self().set_parent(eventLoopTask);
        TDEBUG_END_FUNC_SI_ERR(
          4, "EventProcessor::finishEventAsync", sid, "EXCEPTION");
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
        deferredExceptionPtr_ = current_exception();
      }
      // And then end this task, terminating event processing.
      ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                   sizeof(tbb::internal::task_prefix),
                                 sizeof(tbb::task) +
                                   sizeof(tbb::internal::task_prefix),
                                 "tbb::task");
      tbb::task::self().set_parent(eventLoopTask);
      TDEBUG_END_FUNC_SI_ERR(
        4, "EventProcessor::finishEventAsync", sid, "EXCEPTION");
      return;
    }
    // Create the next event processing task as a continuation
    // of this task, that is transfer our parent, the eventLoopTask,
    // to it.
    processAllEventsAsync(eventLoopTask, sid);
    // And end this task which does not end event loop processing
    // because our parent is the nullptr because we transferred it
    // to the next event processing task.
    TDEBUG_END_FUNC_SI(4, "EventProcessor::finishEventAsync", sid);
  }

  template <Level L>
  void
  EventProcessor::process()
  {
    if ((shutdown_flag > 0) || !ec_->empty()) {
      return;
    }
    ec_->call([this] { begin<L>(); });
    while ((shutdown_flag == 0) && ec_->empty() &&
           levelsToProcess<level_down(L)>()) {
      ec_->call([this] { process<level_down(L)>(); });
    }
    ec_->call([this] {
      finalize<L>();
      recordOutputModuleClosureRequests<L>();
    });
  }

  EventProcessor::StatusCode
  EventProcessor::runToCompletion()
  {
    StatusCode returnCode{epSuccess};
    ec_->call([this, &returnCode] {
      process<highest_level()>();
      if (shutdown_flag > 0) {
        returnCode = epSignal;
      }
    });
    if (!ec_->empty()) {
      terminateAbnormally_();
      ec_->rethrow();
    }
    return returnCode;
  }

  Level
  EventProcessor::advanceItemType()
  {
    auto const itemType = input_->nextItemType();
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
      input_->doBeginJob();
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
    scheduleIteration_.for_each_schedule([this](ScheduleID const sid) {
      schedules_->at(sid).beginJob();
      endPathExecutors_->at(sid).beginJob();
    });
    actReg_->sPostBeginJob.invoke();
    invokePostBeginJobWorkers_();
  }

  void
  EventProcessor::endJob()
  {
    FDEBUG(1) << string(8, ' ') << "endJob\n";
    ec_->call([this] { endJobAllSchedules(); });
    ec_->call([] { ConsumesInfo::instance()->showMissingConsumes(); });
    ec_->call([this] { input_->doEndJob(); });
    ec_->call([this] { actReg_->sPostEndJob.invoke(); });
    ec_->call([] { mf::LogStatistics(); });
    ec_->call([this] {
      detail::writeSummary(pathManager_, scheduler_->wantSummary(), timer_);
    });
  }

  void
  EventProcessor::endJobAllSchedules()
  {
    scheduleIteration_.for_each_schedule([this](ScheduleID const sid) {
      schedules_->at(sid).endJob();
      endPathExecutors_->at(sid).endJob();
    });
  }

  //====================================================
  // File level

  void
  EventProcessor::openInputFile()
  {
    actReg_->sPreOpenFile.invoke();
    FDEBUG(1) << string(8, ' ') << "openInputFile\n";
    fb_.reset(input_->readFile().release());
    if (fb_ == nullptr) {
      throw Exception(errors::LogicError)
        << "Source readFile() did not return a valid FileBlock: FileBlock "
        << "should be valid or readFile() should throw.\n";
    }
    actReg_->sPostOpenFile.invoke(fb_->fileName());
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
    endPathExecutors_->at(ScheduleID::first()).incrementInputFileNumber();
    // Output-file closing on input-file boundaries are tricky since
    // input files must outlive the output files, which often have data
    // copied forward from the input files.  That's why the
    // recordOutputClosureRequests call is made here instead of in a
    // specialization of recordOutputModuleClosureRequests<>.
    endPathExecutors_->at(ScheduleID::first())
      .recordOutputClosureRequests(Granularity::InputFile);
    if (endPathExecutors_->at(ScheduleID::first()).outputsToClose()) {
      closeSomeOutputFiles();
    }
    respondToCloseInputFile();
    actReg_->sPreCloseFile.invoke();
    input_->closeFile();
    actReg_->sPostCloseFile.invoke();
    FDEBUG(1) << string(8, ' ') << "closeInputFile\n";
  }

  void
  EventProcessor::closeAllOutputFiles()
  {
    if (!endPathExecutors_->at(ScheduleID::first()).someOutputsOpen()) {
      return;
    }
    respondToCloseOutputFiles();
    endPathExecutors_->at(ScheduleID::first()).closeAllOutputFiles();
    FDEBUG(1) << string(8, ' ') << "closeAllOutputFiles\n";
  }

  bool
  EventProcessor::outputsToOpen()
  {
    bool outputs_to_open{false};
    auto check_outputs_to_open = [this,
                                  &outputs_to_open](ScheduleID const sid) {
      if (endPathExecutors_->at(sid).outputsToOpen()) {
        outputs_to_open = true;
      }
    };
    scheduleIteration_.for_each_schedule(check_outputs_to_open);
    return outputs_to_open;
  }

  void
  EventProcessor::openSomeOutputFiles()
  {
    if (!outputsToOpen()) {
      return;
    }

    auto open_some_outputs = [this](ScheduleID const sid) {
      endPathExecutors_->at(sid).openSomeOutputFiles(*fb_);
    };
    scheduleIteration_.for_each_schedule(open_some_outputs);

    FDEBUG(1) << string(8, ' ') << "openSomeOutputFiles\n";
    respondToOpenOutputFiles();
  }

  void
  EventProcessor::setOutputFileStatus(OutputFileStatus const ofs)
  {
    endPathExecutors_->at(ScheduleID::first()).setOutputFileStatus(ofs);
    FDEBUG(1) << string(8, ' ') << "setOutputFileStatus\n";
  }

  void
  EventProcessor::closeSomeOutputFiles()
  {
    // Precondition: there are SOME output files that have been
    //               flagged as needing to close.  Otherwise,
    //               'respondtoCloseOutputFiles' will be needlessly
    //               called.
    assert(endPathExecutors_->at(ScheduleID::first()).outputsToClose());
    respondToCloseOutputFiles();
    endPathExecutors_->at(ScheduleID::first()).closeSomeOutputFiles();
    FDEBUG(1) << string(8, ' ') << "closeSomeOutputFiles\n";
  }

  void
  EventProcessor::respondToOpenInputFile()
  {
    scheduleIteration_.for_each_schedule([this](ScheduleID const sid) {
      schedules_->at(sid).respondToOpenInputFile(*fb_);
      endPathExecutors_->at(sid).respondToOpenInputFile(*fb_);
    });
    FDEBUG(1) << string(8, ' ') << "respondToOpenInputFile\n";
  }

  void
  EventProcessor::respondToCloseInputFile()
  {
    scheduleIteration_.for_each_schedule([this](ScheduleID const sid) {
      schedules_->at(sid).respondToCloseInputFile(*fb_);
      endPathExecutors_->at(sid).respondToCloseInputFile(*fb_);
    });
    FDEBUG(1) << string(8, ' ') << "respondToCloseInputFile\n";
  }

  void
  EventProcessor::respondToOpenOutputFiles()
  {
    scheduleIteration_.for_each_schedule([this](ScheduleID const sid) {
      schedules_->at(sid).respondToOpenOutputFiles(*fb_);
      endPathExecutors_->at(sid).respondToOpenOutputFiles(*fb_);
    });
    FDEBUG(1) << string(8, ' ') << "respondToOpenOutputFiles\n";
  }

  void
  EventProcessor::respondToCloseOutputFiles()
  {
    scheduleIteration_.for_each_schedule([this](ScheduleID const sid) {
      schedules_->at(sid).respondToCloseOutputFiles(*fb_);
      endPathExecutors_->at(sid).respondToCloseOutputFiles(*fb_);
    });
    FDEBUG(1) << string(8, ' ') << "respondToCloseOutputFiles\n";
  }

  //=============================================
  // Run level

  void
  EventProcessor::readRun()
  {
    actReg_->sPreSourceRun.invoke();
    runPrincipal_.reset(input_->readRun().release());
    assert(runPrincipal_);
    auto rsh = input_->runRangeSetHandler();
    assert(rsh);
    auto seed_range_set = [this, &rsh](ScheduleID const sid) {
      endPathExecutors_->at(sid).seedRunRangeSet(*rsh);
    };
    scheduleIteration_.for_each_schedule(seed_range_set);
    // The intended behavior here is that the producing services which
    // are called during the sPostReadRun cannot see each others put
    // products. We enforce this by creating the groups for the
    // produced products, but do not allow the lookups to find them
    // until after the callbacks have run.
    runPrincipal_->createGroupsForProducedProducts(
      producedProductLookupTables_);
    psSignals_->sPostReadRun.invoke(*runPrincipal_);
    runPrincipal_->enableLookupOfProducedProducts(producedProductLookupTables_);
    {
      Run const r{*runPrincipal_, invalid_module_context};
      actReg_->sPostSourceRun.invoke(r);
    }
    FDEBUG(1) << string(8, ' ') << "readRun.....................("
              << runPrincipal_->runID() << ")\n";
  }

  void
  EventProcessor::beginRun()
  {
    assert(runPrincipal_);
    RunID const r{runPrincipal_->runID()};
    if (r.isFlush()) {
      return;
    }
    finalizeRunEnabled_ = true;
    try {
      {
        Run const run{*runPrincipal_, invalid_module_context};
        actReg_->sPreBeginRun.invoke(run);
      }
      scheduleIteration_.for_each_schedule([this](ScheduleID const sid) {
        schedules_->at(sid).process(Transition::BeginRun, *runPrincipal_);
        endPathExecutors_->at(sid).process(Transition::BeginRun,
                                           *runPrincipal_);
      });
      {
        Run const run{*runPrincipal_, invalid_module_context};
        actReg_->sPostBeginRun.invoke(run);
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
    assert(runPrincipal_);
    FDEBUG(1) << string(8, ' ') << "setRunAuxiliaryRangeSetID...("
              << runPrincipal_->runID() << ")\n";
    if (endPathExecutors_->at(ScheduleID::first())
          .runRangeSetHandler_.load()
          ->type() == RangeSetHandler::HandlerType::Open) {
      // We are using EmptyEvent source, need to merge
      // what the schedules have seen.
      RangeSet mergedRS;
      auto merge_range_sets = [this, &mergedRS](ScheduleID const sid) {
        auto const rsh = endPathExecutors_->at(sid).runRangeSetHandler_.load();
        auto const& rs = rsh->seenRanges();
        // The following constructor ensures that the range is sorted
        // before 'merge' is called.
        RangeSet const tmp{rs.run(), rs.ranges()};
        mergedRS.merge(tmp);
      };
      scheduleIteration_.for_each_schedule(merge_range_sets);
      runPrincipal_->updateSeenRanges(mergedRS);
      auto update_executors = [this, &mergedRS](ScheduleID const sid) {
        endPathExecutors_->at(sid).setRunAuxiliaryRangeSetID(mergedRS);
      };
      scheduleIteration_.for_each_schedule(update_executors);
      return;
    }

    // Since we are using already existing ranges, all the range set
    // handlers have the same ranges, use the first one.  handler with
    // the largest event number, that will be the one which we will
    // use as the file switch boundary.  Note that is may not match
    // the exactly the schedule that triggered the switch.  Do we need
    // to fix this?
    unique_ptr<RangeSetHandler> rshAtSwitch{
      endPathExecutors_->at(ScheduleID::first())
        .runRangeSetHandler_.load()
        ->clone()};
    if (endPathExecutors_->at(ScheduleID::first()).fileStatus_.load() !=
        OutputFileStatus::Switching) {
      // We are at the end of the job.
      rshAtSwitch->flushRanges();
    }
    runPrincipal_->updateSeenRanges(rshAtSwitch->seenRanges());
    endPathExecutors_->at(ScheduleID::first())
      .setRunAuxiliaryRangeSetID(rshAtSwitch->seenRanges());
  }

  void
  EventProcessor::endRun()
  {
    assert(runPrincipal_);
    // Precondition: The RunID does not correspond to a flush ID. --
    // N.B. The flush flag is not explicitly checked here since endRun
    // is only called from finalizeRun, which is where the check
    // happens.
    RunID const run{runPrincipal_->runID()};
    assert(!run.isFlush());
    try {
      actReg_->sPreEndRun.invoke(runPrincipal_->runID(),
                                 runPrincipal_->endTime());
      scheduleIteration_.for_each_schedule([this](ScheduleID const sid) {
        schedules_->at(sid).process(Transition::EndRun, *runPrincipal_);
        endPathExecutors_->at(sid).process(Transition::EndRun, *runPrincipal_);
      });
      Run const r{*runPrincipal_, invalid_module_context};
      actReg_->sPostEndRun.invoke(r);
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
    assert(runPrincipal_);
    // Precondition: The RunID does not correspond to a flush ID.
    RunID const r{runPrincipal_->runID()};
    assert(!r.isFlush());
    endPathExecutors_->at(ScheduleID::first()).writeRun(*runPrincipal_);
    FDEBUG(1) << string(8, ' ') << "writeRun....................(" << r
              << ")\n";
  }

  //=============================================
  // SubRun level

  void
  EventProcessor::readSubRun()
  {
    actReg_->sPreSourceSubRun.invoke();
    subRunPrincipal_.reset(input_->readSubRun(runPrincipal_.get()).release());
    assert(subRunPrincipal_);
    auto rsh = input_->subRunRangeSetHandler();
    assert(rsh);
    auto seed_range_set = [this, &rsh](ScheduleID const sid) {
      endPathExecutors_->at(sid).seedSubRunRangeSet(*rsh);
    };
    scheduleIteration_.for_each_schedule(seed_range_set);
    // The intended behavior here is that the producing services which are
    // called during the sPostReadSubRun cannot see each others put products. We
    // enforce this by creating the groups for the produced products, but do
    // not allow the lookups to find them until after the callbacks have run.
    subRunPrincipal_->createGroupsForProducedProducts(
      producedProductLookupTables_);
    psSignals_->sPostReadSubRun.invoke(*subRunPrincipal_);
    subRunPrincipal_->enableLookupOfProducedProducts(
      producedProductLookupTables_);
    {
      SubRun const sr{*subRunPrincipal_, invalid_module_context};
      actReg_->sPostSourceSubRun.invoke(sr);
    }
    FDEBUG(1) << string(8, ' ') << "readSubRun..................("
              << subRunPrincipal_->subRunID() << ")\n";
  }

  void
  EventProcessor::beginSubRun()
  {
    assert(subRunPrincipal_);
    SubRunID const sr{subRunPrincipal_->subRunID()};
    if (sr.isFlush()) {
      return;
    }
    finalizeSubRunEnabled_ = true;
    try {
      {
        SubRun const srun{*subRunPrincipal_, invalid_module_context};
        actReg_->sPreBeginSubRun.invoke(srun);
      }
      scheduleIteration_.for_each_schedule([this](ScheduleID const sid) {
        schedules_->at(sid).process(Transition::BeginSubRun, *subRunPrincipal_);
        endPathExecutors_->at(sid).process(Transition::BeginSubRun,
                                           *subRunPrincipal_);
      });
      {
        SubRun const srun{*subRunPrincipal_, invalid_module_context};
        actReg_->sPostBeginSubRun.invoke(srun);
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
    assert(subRunPrincipal_);
    FDEBUG(1) << string(8, ' ') << "setSubRunAuxiliaryRangeSetID("
              << subRunPrincipal_->subRunID() << ")\n";
    if (endPathExecutors_->at(ScheduleID::first())
          .subRunRangeSetHandler_.load()
          ->at(ScheduleID::first())
          ->type() == RangeSetHandler::HandlerType::Open) {
      // We are using EmptyEvent source, need to merge
      // what the schedules have seen.
      RangeSet mergedRS;
      auto merge_range_sets = [this, &mergedRS](ScheduleID const sid) {
        auto const rsh =
          endPathExecutors_->at(sid).subRunRangeSetHandler_.load()->at(
            ScheduleID::first());
        auto const& rs = rsh->seenRanges();
        // The following constructor ensures that the range is sorted
        // before 'merge' is called.
        RangeSet const tmp{rs.run(), rs.ranges()};
        mergedRS.merge(tmp);
      };
      scheduleIteration_.for_each_schedule(merge_range_sets);
      subRunPrincipal_->updateSeenRanges(mergedRS);
      auto update_executors = [this, &mergedRS](ScheduleID const sid) {
        endPathExecutors_->at(sid).setSubRunAuxiliaryRangeSetID(mergedRS);
      };
      scheduleIteration_.for_each_schedule(update_executors);
      return;
    }
    // Ranges are split/flushed only for a RangeSetHandler whose dynamic
    // type is 'ClosedRangeSetHandler'.
    //
    // Consider the following range-sets
    //
    //  SubRun RangeSet:
    //
    //    { Run 1 : SubRun 1 : Events [1,7) }  <-- Current
    //
    //  Run RangeSet:
    //
    //    { Run 1 : SubRun 0 : Events [5,11)
    //              SubRun 1 : Events [1,7)    <-- Current
    //              SubRun 1 : Events [9,15) }
    //
    // For a range split just before SubRun 1, Event 6, the
    // range sets should become:
    //
    //  SubRun RangeSet:
    //
    //    { Run 1 : SubRun 1 : Events [1,6)
    //              SubRun 1 : Events [6,7) } <-- Updated
    //
    //  Run RangeSet:
    //
    //    { Run 1 : SubRun 0 : Events [5,11)
    //              SubRun 1 : Events [1,6)
    //              SubRun 1 : Events [6,7)   <-- Updated
    //              SubRun 1 : Events [9,15) }
    //
    // Since we are using already existing ranges, all the range set
    // handlers have the same ranges.  Find the closed range set
    // handler with the largest event number, that will be the
    // one which we will use as the file switch boundary.  Note
    // that is may not match the exactly the schedule that triggered
    // the switch.  Do we need to fix this?
    //
    // If we do not find any handlers with valid event info then
    // we use the first one, which is just fine.  This happens
    // for example when we are dropping all events.
    //
    unsigned largestEvent = 1U;
    ScheduleID idxOfMax{ScheduleID::first()};
    ScheduleID idx{ScheduleID::first()};
    for (auto& val : *endPathExecutors_->at(ScheduleID::first())
                        .subRunRangeSetHandler_.load()) {
      auto rsh = dynamic_cast<ClosedRangeSetHandler*>(val);
      // Make sure the event number is a valid event number
      // before using it. It can be invalid in the handler if
      // we have not yet read an event, which happens with empty
      // subruns and when we are dropping all events.
      if (rsh->eventInfo().id().isValid() && !rsh->eventInfo().id().isFlush()) {
        if (rsh->eventInfo().id().event() > largestEvent) {
          largestEvent = rsh->eventInfo().id().event();
          idxOfMax = idx;
        }
      }
      idx = idx.next();
    }
    unique_ptr<RangeSetHandler> rshAtSwitch{
      endPathExecutors_->at(ScheduleID::first())
        .subRunRangeSetHandler_.load()
        ->at(idxOfMax)
        ->clone()};
    if (endPathExecutors_->at(ScheduleID::first()).fileStatus_.load() ==
        OutputFileStatus::Switching) {
      rshAtSwitch->maybeSplitRange();
      unique_ptr<RangeSetHandler> runRSHAtSwitch{
        endPathExecutors_->at(idxOfMax).runRangeSetHandler_.load()->clone()};
      runRSHAtSwitch->maybeSplitRange();
      auto& rsh =
        endPathExecutors_->at(ScheduleID::first()).runRangeSetHandler_;
      delete rsh.load();
      rsh = runRSHAtSwitch->clone();
    } else {
      // We are at the end of the job.
      rshAtSwitch->flushRanges();
    }
    for (auto& val : *endPathExecutors_->at(ScheduleID::first())
                        .subRunRangeSetHandler_.load()) {
      delete val;
      val = rshAtSwitch->clone();
    }
    subRunPrincipal_->updateSeenRanges(rshAtSwitch->seenRanges());
    endPathExecutors_->at(ScheduleID::first())
      .setSubRunAuxiliaryRangeSetID(rshAtSwitch->seenRanges());
  }

  void
  EventProcessor::endSubRun()
  {
    assert(subRunPrincipal_);
    // Precondition: The SubRunID does not correspond to a flush ID.
    // Note: the flush flag is not explicitly checked here since
    // endSubRun is only called from finalizeSubRun, which is where the
    // check happens.
    SubRunID const sr{subRunPrincipal_->subRunID()};
    assert(!sr.isFlush());
    try {
      actReg_->sPreEndSubRun.invoke(subRunPrincipal_->subRunID(),
                                    subRunPrincipal_->endTime());
      scheduleIteration_.for_each_schedule([this](ScheduleID const sid) {
        schedules_->at(sid).process(Transition::EndSubRun, *subRunPrincipal_);
        endPathExecutors_->at(sid).process(Transition::EndSubRun,
                                           *subRunPrincipal_);
      });
      SubRun const srun{*subRunPrincipal_, invalid_module_context};
      actReg_->sPostEndSubRun.invoke(srun);
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
    assert(subRunPrincipal_);
    // Precondition: The SubRunID does not correspond to a flush ID.
    SubRunID const& sr{subRunPrincipal_->subRunID()};
    assert(!sr.isFlush());
    endPathExecutors_->at(ScheduleID::first()).writeSubRun(*subRunPrincipal_);
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
