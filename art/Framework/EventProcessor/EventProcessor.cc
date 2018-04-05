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
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Framework/Services/System/FloatingPointControl.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Utilities/CPCSentry.h"
#include "art/Utilities/CurrentProcessingContext.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/Transition.h"
#include "art/Utilities/bold_fontify.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/ostream_handle.h"
#include "fhiclcpp/types/detail/validationException.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskList.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "tbb/task.h"
#include "tbb/task_arena.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <condition_variable>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

using namespace art;
using namespace hep::concurrency;
using namespace std;
using namespace literals::string_literals;

using fhicl::ParameterSet;

namespace {

  unique_ptr<InputSource>
  makeInput(ParameterSet const& params,
            string const& processName,
            UpdateOutputCallbacks& callbacks,
            ActivityRegistry& areg)
  {
    unique_ptr<InputSource> source;
    ParameterSet defaultEmptySource;
    defaultEmptySource.put("module_type", "EmptyEvent");
    defaultEmptySource.put("module_label", "source");
    defaultEmptySource.put("maxEvents", -1);
    // find single source
    bool sourceSpecified{false};
    ParameterSet main_input{defaultEmptySource};
    try {
      if (!params.get_if_present("source", main_input)) {
        mf::LogInfo("EventProcessorSourceConfig")
          << "Could not find a source configuration: using default.";
      }
      // Fill in "ModuleDescription", in case the input source
      // produces any EDproducts, which would be registered in the
      // UpdateOutputCallbacks.  Also fill in the process history item
      // for this process.
      ModuleDescription const md{
        main_input.id(),
        main_input.get<string>("module_type"),
        main_input.get<string>("module_label"),
        static_cast<int>(ModuleThreadingType::LEGACY),
        ProcessConfiguration{processName, params.id(), getReleaseVersion()}};
      sourceSpecified = true;
      InputSourceDescription isd{md, callbacks, areg};
      try {
        source = InputSourceFactory::make(main_input, isd);
      }
      catch (fhicl::detail::validationException const& e) {
        throw Exception(errors::Configuration)
          << "\n\nModule label: " << detail::bold_fontify(md.moduleLabel())
          << "\nmodule_type : " << detail::bold_fontify(md.moduleName())
          << "\n\n"
          << e.what();
      }
      return source;
    }
    catch (Exception const& x) {
      if ((sourceSpecified == false) &&
          (errors::Configuration == x.categoryCode())) {
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
    // FIXME: We should not try to run without an input source!
    // FIXME: We should have a catch (...) clause to thow something instead of
    // returning no source at all.
    return unique_ptr<InputSource>();
  }

  inline string
  spaces(unsigned const n)
  {
    return string(n, ' ');
  }

  class Waiter : public tbb::task {
  public:
    tbb::task*
    execute()
    {
      return nullptr;
    }
  };

} // unnamed namespace

EventProcessor::~EventProcessor()
{
  // Services must stay usable until they go out of scope, meaning
  // that modules may (say) use services in their destructors.
  // servicesActivate_(serviceToken_);
}

EventProcessor::EventProcessor(ParameterSet const& pset)
  : act_table_{pset.get<ParameterSet>("services.scheduler")}
  , actReg_()
  , mfStatusUpdater_{actReg_}
  , servicesManager_{initServices_(pset, actReg_)}
  , pathManager_{pset,
                 outputCallbacks_,
                 productsToProduce_,
                 act_table_,
                 actReg_}
  , handleEmptyRuns_{pset.get<bool>("services.scheduler.handleEmptyRuns", true)}
  , handleEmptySubRuns_{
      pset.get<bool>("services.scheduler.handleEmptySubRuns", true)}
{
  // Initialize TBB with desired number of threads.
  auto nthreads = pset.get<int>("services.scheduler.num_threads");
  tbbManager_.initialize(nthreads);
  mf::LogInfo("MTdiagnostics")
    << "TBB has been configured to use a maximum of "
    << tbb::this_task_arena::max_concurrency() << " threads.";
  Globals::instance()->setNThreads(nthreads);

  auto const nschedules = eventPrincipal_.expand_to_num_schedules();
  {
    ostringstream buf;
    buf << "-----> EventProcessor::EventProcessor: nschedules: " << nschedules
        << " nthreads: " << nthreads << "\n";
    TDEBUG(5) << buf.str();
  }
  auto errorOnMissingConsumes =
    pset.get<bool>("services.scheduler.errorOnMissingConsumes", false);
  ConsumesInfo::instance()->setRequireConsumes(errorOnMissingConsumes);
  string const& processName{pset.get<string>("process_name")};
  servicesManager_->addSystemService<TriggerNamesService>(
    pset, pathManager_.triggerPathNames());
  // We have delayed creating the service module instances until
  // now.  Now actually create them.
  servicesManager_->forceCreation();
  ProcessConfiguration const pc{processName, pset.id(), getReleaseVersion()};
  servicesManager_->registerProducts(productsToProduce_, psSignals_, pc);

  ServiceHandle<FileCatalogMetadata> {}
  ->addMetadataString("process_name", processName);

  pathManager_.createModulesAndWorkers();
  endPathExecutor_ = make_unique<EndPathExecutor>(
    pathManager_, act_table_, actReg_, outputCallbacks_);
  for (auto sid = ScheduleID::first(); sid < ScheduleID(nschedules);
       sid = sid.next()) {
    schedule_.emplace_back(sid,
                           pathManager_,
                           processName,
                           pset,
                           outputCallbacks_,
                           productsToProduce_,
                           act_table_,
                           actReg_);
  }
  FDEBUG(2) << pset.to_string() << endl;

  // The input source must be made *after* the end-path executor has
  // been made: the end-path executor registers a callback that must
  // be invoked once the first input file is opened.
  input_ = makeInput(pset, processName, outputCallbacks_, actReg_);
  actReg_.sPostSourceConstruction.invoke(input_->moduleDescription());

  // Create product tables used for product retrieval within modules.
  producedProducts_ = ProductTables{productsToProduce_};
  outputCallbacks_.invoke(producedProducts_);
}

ServicesManager*
EventProcessor::initServices_(ParameterSet const& topPSet,
                              ActivityRegistry& areg)
{
  auto servicesPSet = topPSet.get<ParameterSet>("services", {});
  auto const fpcPSet =
    servicesPSet.get<ParameterSet>("floating_point_control", {});
  servicesPSet.erase("floating_point_control");
  servicesPSet.erase("message");
  auto mgr = new ServicesManager(move(servicesPSet), areg);
  ServiceRegistry::instance().setManager(mgr);
  mgr->addSystemService<FloatingPointControl>(fpcPSet, areg);
  return mgr;
}

void
EventProcessor::invokePostBeginJobWorkers_()
{
  // Need to convert multiple lists of workers into a long list that
  // the postBeginJobWorkers callbacks can understand.
  vector<Worker*> allWorkers;
  {
    auto const& workers =
      pathManager_.triggerPathsInfo(ScheduleID::first()).workers();
    cet::transform_all(workers,
                       back_inserter(allWorkers),
                       [](auto const& label_And_worker) {
                         return label_And_worker.second;
                       });
  }
  {
    auto const& workers = pathManager_.endPathInfo().workers();
    cet::transform_all(workers,
                       back_inserter(allWorkers),
                       [](auto const& label_And_worker) {
                         return label_And_worker.second;
                       });
  }
  actReg_.sPostBeginJobWorkers.invoke(input_.get(), allWorkers);
}

//================================================================
// Event-loop infrastructure

template <Level L>
bool
EventProcessor::levelsToProcess()
{
  if (nextLevel_ == Level::ReadyToAdvance) {
    nextLevel_ = advanceItemType();
    // Consider reading right here?
  }
  if (nextLevel_ == L) {
    nextLevel_ = Level::ReadyToAdvance;
    if (endPathExecutor_->outputsToClose()) {
      setOutputFileStatus(OutputFileStatus::Switching);
      finalizeContainingLevels<L>();
      closeSomeOutputFiles();
    }
    return true;
  } else if (nextLevel_ < L) {
    return false;
  } else if (nextLevel_ == highest_level()) {
    return false;
  }
  throw Exception{errors::LogicError} << "Incorrect level hierarchy.";
}

namespace art {

  // Specializations for process function template

  template <>
  inline void
  EventProcessor::begin<Level::Job>()
  {
    timer_.start();
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
    if (nextLevel_ == Level::Job) {
      closeAllFiles();
    } else {
      closeInputFile();
    }
  }

  template <>
  inline void
  EventProcessor::finalize<Level::Job>()
  {
    endJob();
    timer_.stop();
  }

  template <>
  inline void
  EventProcessor::finalizeContainingLevels<Level::SubRun>()
  {
    finalize<Level::Run>();
  }

  template <>
  inline void
  EventProcessor::finalizeContainingLevels<Level::Event>()
  {
    finalize<Level::SubRun>();
    finalize<Level::Run>();
  }

  template <>
  inline void
  EventProcessor::recordOutputModuleClosureRequests<Level::Run>()
  {
    endPathExecutor_->recordOutputClosureRequests(Granularity::Run);
  }

  template <>
  inline void
  EventProcessor::recordOutputModuleClosureRequests<Level::SubRun>()
  {
    endPathExecutor_->recordOutputClosureRequests(Granularity::SubRun);
  }

  template <>
  inline void
  EventProcessor::recordOutputModuleClosureRequests<Level::Event>()
  {
    endPathExecutor_->recordOutputClosureRequests(Granularity::Event);
  }

  template <>
  void
  EventProcessor::process<most_deeply_nested_level()>()
  {
    auto const nschedules = Globals::instance()->nschedules();
    if ((shutdown_flag > 0) || !ec_.empty()) {
      return;
    }
    firstEvent_ = true;
    tbb::task_group_context tgc(tbb::task_group_context::isolated);
    // FIXME: Hacked this in while trying to get thread-safe
    // FIXME: file output switching working.  This should be
    // FIXME: somewhere else probably!!!
    // Note: This loop is to allow output file switching to
    // happen in the main thread.
    bool done = false;
    while (!done) {
      beginRunIfNotDoneAlready();
      beginSubRunIfNotDoneAlready();
      // Note: This returns a unique_ptr to an EmptyTask with
      // a custom destructor that destroys the task.
      auto EventLoopTask = new (tbb::task::allocate_root()) Waiter;
      EventLoopTask->change_group(tgc);
      EventLoopTask->set_ref_count(nschedules + 1);
      auto si = ScheduleID::first();
      tbb::task_list schedule_heads;
      for (; si < ScheduleID(nschedules); si = si.next()) {
        auto processAllEventsFunctor = [this, si](exception_ptr const*) {
          processAllEventsAsync(si);
        };
        schedule_heads.push_back(*make_waiting_task(
          EventLoopTask->allocate_child(), processAllEventsFunctor));
      }
      // FIXME: threading: spawn_and_wait_for_all has the nasty
      // FIXME: threading: habit of consuming lots of cpu time
      // FIXME: threading: in a spin wait.  Replace with a semaphore.
      EventLoopTask->spawn_and_wait_for_all(schedule_heads);
      tbb::task::destroy(*EventLoopTask);
      // If anything bad happened during event processing,
      // let the user know.
      if (deferredExceptionPtrIsSet_) {
        rethrow_exception(deferredExceptionPtr_);
      }
      if (!fileSwitchInProgress_) {
        done = true;
        continue;
      }
      setOutputFileStatus(OutputFileStatus::Switching);
      finalizeContainingLevels<most_deeply_nested_level() /*Event*/>();
      // FIXME: Need to notify all schedules!
      respondToCloseOutputFiles();
      {
        endPathExecutor_->closeSomeOutputFiles();
        FDEBUG(1) << spaces(8) << "closeSomeOutputFiles\n";
      }
      // FIXME: Need to notify all schedules!
      // We started the switch after advancing the file index
      // iterator, we must make sure that we read that event
      // before advancing the iterator again.
      firstEvent_ = true;
      fileSwitchInProgress_ = false;
    }
  }

} // namespace art

// Spawn the Process All Events Task (the event loop).
// It makes a continuation task which reads and processes a single
// event, creates itself again as a continuation task, and then exits.
void
EventProcessor::processAllEventsAsync(ScheduleID const si)
{
  TDEBUG(4) << "-----> Begin EventProcessor::processAllEventsAsync (" << si
            << ") ...\n";
  auto readAndProcessEventFunctor = [this, si](exception_ptr const*) {
    // Note: When we come here our parent is the eventLoop task.
    TDEBUG(4) << "=====> Begin readAndProcessEventTask (" << si << ") ...\n";
    try {
      processAllEventsAsync_readAndProcess(si);
    }
    catch (...) {
      // Event processing threw an exception.
      // Use a thread-safe, one-way trapdoor pattern to notify
      // the main thread.
      bool expected = false;
      if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected, true)) {
        // Put the exception where the main thread can get at it.
        deferredExceptionPtr_ = current_exception();
      }
      // And then end this task, terminating event processing.
      TDEBUG(4) << "=====> End   readAndProcessEventTask (" << si
                << ") ... terminate event loop because of EXCEPTION\n";
      return;
    }
    // And then end this task, which does not terminate event
    // processing because our parent is the nullptr because
    // we transferred it to another task.
    TDEBUG(4) << "=====> End   readAndProcessEventTask (" << si << ") ...\n";
    return;
  };
  // Create a task that has the EventLoopTask as parent,
  // and reset our parent to the nullptr, which means when
  // we end we do not decrement the ref count of the EventLoopTask.
  auto readAndProcessEventTask = make_waiting_task(
    tbb::task::self().allocate_continuation(), readAndProcessEventFunctor);
  tbb::task::spawn(*readAndProcessEventTask);
  // And end this task, which does not terminate event processing
  // because our parent is the nullptr because we transferred it
  // to the readAndProcessEventTask.
  TDEBUG(4) << "-----> End   EventProcessor::processAllEventsAsync (" << si
            << ") ...\n";
}

// This function is executed as the main body of the readAndProcessEvent task,
// our parent process is the EventLoopTask.
// Here we advance to the next item in the file index, end event processing
// if it is not an event, or if the user has requested a shutdown, read the
// event, and then call another function to do the processing.
void
EventProcessor::processAllEventsAsync_readAndProcess(ScheduleID const si)
{
  TDEBUG(4)
    << "-----> Begin EventProcessor::processAllEventsAsync_readAndProcess ("
    << si << ") ...\n";
  // Note: shutdown_flag is a extern global atomic int in
  // art/art/Utilities/UnixSignalHandlers.cc
  if (shutdown_flag) {
    // User called for a clean shutdown using a signal or ctrl-c,
    // end event processing and this task.
    TDEBUG(4)
      << "-----> End   EventProcessor::processAllEventsAsync_readAndProcess ("
      << si << ") ... CLEAN SHUTDOWN\n";
    return;
  }
  processAllEventsAsync_readAndProcess_after_possible_output_switch(si);
  TDEBUG(4)
    << "-----> End   EventProcessor::processAllEventsAsync_readAndProcess ("
    << si << ") ...\n";
}

// This function is a continuation of the body of the readAndProcessEvent task.
void
EventProcessor::
  processAllEventsAsync_readAndProcess_after_possible_output_switch(
    ScheduleID const si)
{
  TDEBUG(4) << "-----> Begin "
               "EventProcessor::processAllEventsAsync_readAndProcess_after_"
               "possible_output_switch ("
            << si << ") ...\n";
  // The item type advance and the event read must be done with the
  // input source lock held, however the event processing should not
  // be.
  auto& ep = eventPrincipal_[si];
  {
    // FIXME: By using a recursive mutex here we are assuming
    // FIXME: that all the other schedules are definitely running
    // FIXME: on other threads!  This can be broken by task
    // FIXME: stealing!!!
    lock_guard<recursive_mutex> lock_input(
      *SharedResourcesRegistry::instance()->getMutexForSource());
    if (fileSwitchInProgress_) {
      // We must avoid advancing the iterator after a schedule has
      // noticed it is time to switch files.  After the switch, we
      // will need to set firstEvent_ true so that the first schedule
      // that resumes after the switch actually reads the event that
      // the first schedule which noticed we needed a switch had
      // advanced the iterator to.

      // Note: We still have the problem that because the schedules do
      // not read events at the same time the file switch point can be
      // up to #schedules-1 ahead of where it would have been if there
      // was only one schedule.  If we are switching output files every
      // event in an attempt to create single event files, this really
      // does not work out too well.
      TDEBUG(4) << "-----> End   "
                   "EventProcessor::processAllEventsAsync_readAndProcess ("
                << si << ") ...\n";
      return;
    }
    //
    //  Check the file index for what comes next and exit this task if
    //  it is not an event, or if the user has asynchronously
    //  requested a shutdown.
    //
    auto expected = true;
    if (firstEvent_.compare_exchange_strong(expected, false)) {
      // Do not advance the item type on the first event.
    } else {
      // Do the advance item type.
      if (nextLevel_ == Level::ReadyToAdvance) {
        // See what the next item is.
        TDEBUG(5) << "-----> End   "
                     "EventProcessor::processAllEventsAsync_readAndProcess_"
                     "after_possible_output_switch ("
                  << si << ") ... Calling advanceItemType()\n";
        nextLevel_ = advanceItemType();
      }
      if ((nextLevel_ < most_deeply_nested_level() /*Event*/) ||
          (nextLevel_ == highest_level() /*File*/)) {
        // We are popping up, end event processing and this task.
        TDEBUG(4) << "-----> End   "
                     "EventProcessor::processAllEventsAsync_readAndProcess_"
                     "after_possible_output_switch ("
                  << si << ") ... END OF SUBRUN\n";
        return;
      }
      if (nextLevel_ != most_deeply_nested_level() /*Event*/) {
        // Error: incorrect level hierarchy
        TDEBUG(4) << "-----> End   "
                     "EventProcessor::processAllEventsAsync_readAndProcess_"
                     "after_possible_output_switch ("
                  << si << ") ... BAD HIERARCHY\n";
        throw Exception{errors::LogicError} << "Incorrect level hierarchy.";
      }
      nextLevel_ = Level::ReadyToAdvance;
      // At this point we have determined that we are going to read an event
      // and we must do that before dropping the lock on the input source
      // which is what is protecting us against a double-advance caused by
      // a different schedule.
      if (endPathExecutor_->outputsToClose()) {
        TDEBUG(5)
          << "-----> EventProcessor::processAllEventsAsync_readAndProcess ("
          << si << ") ... there are outputs to close\n";
        fileSwitchInProgress_ = true;
        TDEBUG(4) << "-----> End   "
                     "EventProcessor::processAllEventsAsync_readAndProcess ("
                  << si << ") ...\n";
        return;
      }
    }
    //
    //  Now we can read the event from the source.
    //
    // readEvent();
    {
      assert(subRunPrincipal_);
      assert(subRunPrincipal_->subRunID().isValid());
      {
        CurrentProcessingContext cpc{si, nullptr, -1, false};
        detail::CPCSentry sentry{cpc};
        actReg_.sPreSourceEvent.invoke();
        TDEBUG(5)
          << "-----> "
             "EventProcessor::processAllEventsAsync_readAndProcess_after_"
             "possible_output_switch ("
          << si << ") ... Calling input_->readEvent(subRunPrincipal_.get())\n";
        ep = input_->readEvent(subRunPrincipal_.get());
        assert(ep);
        ep->enableProductCreation(producedProducts_);
        psSignals_.sPostReadEvent.invoke(*ep);

        ep->setProducedProducts(producedProducts_);
        Event const e{*ep, ModuleDescription{}, TypeLabelLookup_t{}};
        actReg_.sPostSourceEvent.invoke(e);
      }
      assert(ep);
      FDEBUG(1) << spaces(8) << "readEvent...................(" << ep->eventID()
                << ")\n";
    }
    // Now we drop the input source lock by exiting the guarded scope.
  }
  if (ep->eventID().isFlush()) {
    // No processing to do, start next event handling task,
    // transferring our parent task (EventLoopTask) to it,
    // and exit this task.
    processAllEventsAsync(si);
    TDEBUG(4) << "-----> End   "
                 "EventProcessor::processAllEventsAsync_readAndProcess_after_"
                 "possible_output_switch ("
              << si << ") ... FLUSH EVENT\n";
    return;
  }
  // Now process the event.
  processAllEventsAsync_processEvent(si);
  // And end this task, which does not terminate event processing
  // because our parent is the nullptr because we transferred it
  // to the endPathTask.
  TDEBUG(4) << "-----> End   "
               "EventProcessor::processAllEventsAsync_readAndProcess_after_"
               "possible_output_switch ("
            << si << ") ...\n";
}

// This function is a continuation of the body of the readAndProcessEvent task.
// Here we call down to Schedule to do the trigger path processing, passing
// it a waiting task which will do the end path processing, finalize the event,
// and start the next read and process event task.  Note that Schedule will
// spawn a task to process each of the trigger paths, and then when they are
// finished, insert the trigger results, and then spawn the waiting task we
// gave it to do the end path processing, write the event, and then start the
// next event processing task.
void
EventProcessor::processAllEventsAsync_processEvent(ScheduleID const si)
{
  TDEBUG(4)
    << "-----> Begin EventProcessor::processAllEventsAsync_processEvent (" << si
    << ") ...\n";
  auto eventLoopTask = tbb::task::self().parent();
  {
    assert(eventPrincipal_[si]);
    assert(!eventPrincipal_[si]->eventID().isFlush());
    {
      try {
        auto endPathFunctor = [this, si](exception_ptr const* ex) {
          // Note: When we start our parent is the eventLoopTask.
          TDEBUG(4) << "=====> Begin endPathTask (" << si << ") ...\n";
          if (ex != nullptr) {
            try {
              rethrow_exception(*ex);
            }
            catch (cet::exception& e) {
              if (act_table_.find(e.root_cause()) !=
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
                  deferredExceptionPtr_ = ex_ptr;
                }
                // And then end this task, terminating event processing.
                TDEBUG(4) << "=====> End   endPathTask ... terminate event "
                             "loop because of EXCEPTION\n";
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
              TDEBUG(4) << "=====> End   endPathTask ... terminate event loop "
                           "because of EXCEPTION\n";
              return;
            }
            // WARNING: We only get here if the trigger paths threw
            // and we are ignoring the exception because of
            // actions::IgnoreCompletely.
          }
          processAllEventsAsync_processEndPath(si);
          // And end this task, which does not terminate event
          // processing because our parent is the nullptr because
          // it got transferred to the next process event task.
          TDEBUG(4) << "=====> End   endPathTask (" << si << ")...\n";
          return;
        };
        // Make the end path processing task, make its parent
        // the EventLoopTask, and set our parent to the nullptr
        // so we can exit without ending event processing.
        auto endPathTask = make_waiting_task(
          tbb::task::self().allocate_continuation(), endPathFunctor);
        {
          Event const ev{
            *eventPrincipal_[si], ModuleDescription{}, TypeLabelLookup_t{}};
          CurrentProcessingContext cpc{si, nullptr, -1, false};
          detail::CPCSentry sentry{cpc};
          actReg_.sPreProcessEvent.invoke(ev);
        }
        // Start the trigger paths running.  When they finish
        // they will spawn the endPathTask which will run the
        // end path, write the event, and start the next event
        // processing task.
        schedule_[si].process_event(endPathTask, *eventPrincipal_[si], si);
        // Once the trigger paths are running we are done, exit this task,
        // which does not end event processing because our parent is the
        // nullptr because we transferred it to the endPathTask above.
        TDEBUG(4) << "-----> End   "
                     "EventProcessor::processAllEventsAsync_processEvent ("
                  << si << ") ...\n";
        return;
      }
      catch (cet::exception& e) {
        if (act_table_.find(e.root_cause()) != actions::IgnoreCompletely) {
          auto ex_ptr = make_exception_ptr(
            Exception{errors::EventProcessorFailure,
                      "EventProcessor: an exception occurred during current "
                      "event processing",
                      e});
          // Use a thread-safe, one-way trapdoor pattern to notify the main
          // thread of the exception.
          bool expected = false;
          if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected,
                                                                 true)) {
            // Put the exception where the main thread can get at it.
            deferredExceptionPtr_ = ex_ptr;
          }
          // Do this in case we already gave our parent to
          // to endPathTask.
          tbb::task::self().set_parent(eventLoopTask);
          // And then end this task, terminating event processing.
          TDEBUG(4) << "-----> End   "
                       "EventProcessor::processAllEventsAsync_processEvent ("
                    << si
                    << ") ... terminate event loop because of EXCEPTION\n";
          return;
        }
        mf::LogWarning(e.category())
          << "exception being ignored for current event:\n"
          << cet::trim_right_copy(e.what(), " \n");
        // Do this in case we already gave our parent to
        // to endPathTask.
        tbb::task::self().set_parent(eventLoopTask);
        // WARNING: We continue processing after the catch blocks!!!
      }
      catch (...) {
        mf::LogError("PassingThrough")
          << "an exception occurred during current event processing\n";
        // Use a thread-safe, one-way trapdoor pattern to notify the main thread
        // of the exception.
        bool expected = false;
        if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected,
                                                               true)) {
          // Put the exception where the main thread can get at it.
          deferredExceptionPtr_ = current_exception();
        }
        // Do this in case we already gave our parent to
        // to endPathTask.
        tbb::task::self().set_parent(eventLoopTask);
        // And then end this task, terminating event processing.
        TDEBUG(4) << "-----> End   "
                     "EventProcessor::processAllEventsAsync_processEvent ("
                  << si << ") ... terminate event loop because of EXCEPTION\n";
        return;
      }
    }
  }
  // WARNING: The only way to get here is if starting the trigger path
  // processing threw and actions::IgnoreCompletely is set!
  processAllEventsAsync_finishEvent(si);
  // And then end this task, which does not end event
  // processing because we finishEvent transferred our
  // parent to the next event processing task.
  TDEBUG(4)
    << "-----> End   EventProcessor::processAllEventsAsync_processEvent (" << si
    << ") ...\n";
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
// the schedules end when we push onto the queue, and are recreated
// after the event is written to process the next event.
void
EventProcessor::processAllEventsAsync_processEndPath(ScheduleID const si)
{
  TDEBUG(4)
    << "-----> Begin EventProcessor::processAllEventsAsync_processEndPath ("
    << si << ") ...\n";
  auto eventLoopTask = tbb::task::self().parent();
  // Arrange it so that we can end the task without
  // terminating event processing.
  tbb::task::self().set_parent(nullptr);
  auto endPathFunctor = [this, si, eventLoopTask] {
    auto& ep = eventPrincipal_[si];
    TDEBUG(4) << "=====> Begin "
                 "EventProcessor::processAllEventsAsync_processEndPath::"
                 "endPathFunctor ("
              << si << ") ...\n";
    // Arrange it so that we can terminate event
    // processing if we want to.
    tbb::task::self().set_parent(eventLoopTask);
    try {
      endPathExecutor_->process_event(*ep, si);
    }
    catch (cet::exception& e) {
      // Possible actions: IgnoreCompletely, Rethrow, SkipEvent, FailModule,
      // FailPath
      if (act_table_.find(e.root_cause()) != actions::IgnoreCompletely) {
        // Possible actions: Rethrow, SkipEvent, FailModule, FailPath
        auto ex_ptr = make_exception_ptr(
          Exception{errors::EventProcessorFailure,
                    "EventProcessor: an exception occurred during current "
                    "event processing",
                    e});
        // Use a thread-safe, one-way trapdoor pattern to
        // notify the main thread of the exception.
        bool expected = false;
        if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected,
                                                               true)) {
          // Put the exception where the main thread can get at it.
          deferredExceptionPtr_ = ex_ptr;
        }
        // And then end this task, terminating event processing.
        TDEBUG(4) << "=====> End   "
                     "EventProcessor::processAllEventsAsync_processEndPath::"
                     "endPathFunctor ("
                  << si << ") ... terminate event loop because of EXCEPTION\n";
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
        deferredExceptionPtr_ = current_exception();
      }
      // And then end this task, terminating event processing.
      TDEBUG(4) << "=====> End   "
                   "EventProcessor::processAllEventsAsync_processEndPath::"
                   "endPathFunctor ("
                << si << ") ... terminate event loop because of EXCEPTION\n";
      return;
    }
    {
      Event const ev{*ep, ModuleDescription{}, TypeLabelLookup_t{}};
      CurrentProcessingContext cpc{si, nullptr, -1, false};
      detail::CPCSentry sentry{cpc};
      actReg_.sPostProcessEvent.invoke(ev);
    }
    processAllEventsAsync_finishEvent(si);
    // Note that we do not terminate event processing when we
    // end because processAllEventsAsync_finishEvent has set
    // our parent to the nullptr.
    TDEBUG(4) << "=====> End   "
                 "EventProcessor::processAllEventsAsync_processEndPath::"
                 "endPathFunctor ("
              << si << ") ...\n";
  };
  endPathExecutor_->serialTaskQueue().push(endPathFunctor);
  // Once the end path processing and event finalization processing
  // is queued we are done, exit this task, which does not end event
  // processing because our parent is the nullptr because we transferred
  // it to the endPathFunctor.
  TDEBUG(4)
    << "-----> End   EventProcessor::processAllEventsAsync_processEndPath ("
    << si << ") ...\n";
}

// This function is a continuation of the Process End Path task, or
// the error handling for an ignored trigger path exception in the
// Process All Events task, or an ignored end path exception in the
// Process End Path task.  Our parent task is the eventLoopTask.
// We write the event out, spawn the next event processing task, and
// end the current task.
void
EventProcessor::processAllEventsAsync_finishEvent(ScheduleID const si)
{
  auto& ep = eventPrincipal_[si];
  TDEBUG(4)
    << "-----> Begin EventProcessor::processAllEventsAsync_finishEvent (" << si
    << ") ...\n";
  FDEBUG(1) << spaces(8) << "processEvent................(" << ep->eventID()
            << ")\n";
  try {
    // Ask the output workers if they have reached their limits,
    // and if so setup to end the job the next time around the
    // event loop.
    FDEBUG(1) << spaces(8) << "shouldWeStop\n";
    TDEBUG(5) << "-----> EventProcessor::processAllEventsAsync_finishEvent ("
              << si << ") ... Calling endPathExecutor_->allAtLimit()\n";
    if (endPathExecutor_->allAtLimit()) {
      nextLevel_ = highest_level() /*File*/;
    }
    //
    //  Now we can write the results of processing
    //  to the outputs.
    //
    {
      assert(ep);
      if (!ep->eventID().isFlush()) {
        {
          TDEBUG(5)
            << "-----> EventProcessor::processAllEventsAsync_finishEvent ("
            << si << ") ... calling endPathExecutor_->outputsToOpen()\n";
          if (endPathExecutor_->outputsToOpen()) {
            TDEBUG(5)
              << "-----> EventProcessor::processAllEventsAsync_finishEvent ("
              << si
              << ") ... calling "
                 "endPathExecutor_->openSomeOutputFiles(*fb_)\n";
            endPathExecutor_->openSomeOutputFiles(*fb_);
            FDEBUG(1) << spaces(8) << "openSomeOutputFiles\n";
            respondToOpenOutputFiles();
          }
        }
        {
          assert(ep);
          assert(!ep->eventID().isFlush());
          TDEBUG(5)
            << "-----> EventProcessor::processAllEventsAsync_finishEvent ("
            << si
            << ") ... calling endPathExecutor_->writeEvent(si, "
               "*ep)\n";
          endPathExecutor_->writeEvent(si, *ep);
          FDEBUG(1) << spaces(8) << "writeEvent..................("
                    << ep->eventID() << ")\n";
          ep.reset();
        }
      }
    }
    {
      TDEBUG(5) << "-----> EventProcessor::processAllEventsAsync_finishEvent ("
                << si
                << ") ... calling "
                   "endPathExecutor_->recordOutputClosureRequests(Granularity::"
                   "Event)\n";
      endPathExecutor_->recordOutputClosureRequests(Granularity::Event);
    }
  }
  catch (cet::exception& e) {
    if (act_table_.find(e.root_cause()) != actions::IgnoreCompletely) {
      auto ex_ptr =
        make_exception_ptr(Exception{errors::EventProcessorFailure,
                                     "EventProcessor: an exception occurred "
                                     "during current event processing",
                                     e});
      // Use a thread-safe, one-way trapdoor pattern to notify the main thread
      // of the exception.
      bool expected = false;
      if (deferredExceptionPtrIsSet_.compare_exchange_strong(expected, true)) {
        // Put the exception where the main thread can get at it.
        deferredExceptionPtr_ = ex_ptr;
      }
      // And then end this task, terminating event processing.
      TDEBUG(4)
        << "-----> End   EventProcessor::processAllEventsAsync_finishEvent ("
        << si << ") ... due to exception\n";
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
    TDEBUG(4)
      << "-----> End   EventProcessor::processAllEventsAsync_finishEvent ("
      << si << ") ... due to exception\n";
    return;
  }
  // Create the next event processing task as a continuation
  // of this task, that is transfer our parent, the eventLoopTask,
  // to it.
  processAllEventsAsync(si);
  // And end this task which does not end event loop processing
  // because our parent is the nullptr because we transferred it
  // to the next event processing task.
  TDEBUG(4)
    << "-----> End   EventProcessor::processAllEventsAsync_finishEvent (" << si
    << ") ...\n";
}

template <Level L>
void
EventProcessor::process()
{
  if ((shutdown_flag > 0) || !ec_.empty()) {
    return;
  }
  ec_.call([this] { begin<L>(); });
  while ((shutdown_flag == 0) && ec_.empty() &&
         levelsToProcess<level_down(L)>()) {
    ec_.call([this] { process<level_down(L)>(); });
  }
  ec_.call([this] {
    finalize<L>();
    recordOutputModuleClosureRequests<L>();
  });
}

EventProcessor::StatusCode
EventProcessor::runToCompletion()
{
  StatusCode returnCode{epSuccess};
  ec_.call([this, &returnCode] {
    process<highest_level()>();
    if (shutdown_flag > 0) {
      returnCode = epSignal;
    }
  });
  if (!ec_.empty()) {
    terminateAbnormally_();
    ec_.rethrow();
  }
  return returnCode;
}

Level
EventProcessor::advanceItemType()
{
  auto const itemType = input_->nextItemType();
  FDEBUG(1) << spaces(4) << "*** nextItemType: " << itemType << " ***\n";
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
  FDEBUG(1) << spaces(8) << "beginJob\n";
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
  schedule_[ScheduleID::first()].beginJob();
  endPathExecutor_->beginJob();
  actReg_.sPostBeginJob.invoke();
  invokePostBeginJobWorkers_();
}

void
EventProcessor::endJob()
{
  FDEBUG(1) << spaces(8) << "endJob\n";
  ec_.call([this] { schedule_[ScheduleID::first()].endJob(); });
  ec_.call([this] { endPathExecutor_->endJob(); });
  ec_.call([] { ConsumesInfo::instance()->showMissingConsumes(); });
  ec_.call([this] { input_->doEndJob(); });
  ec_.call([this] { actReg_.sPostEndJob.invoke(); });
  ec_.call([this] {
    detail::writeSummary(
      pathManager_,
      ServiceHandle<TriggerNamesService const> {}->wantSummary(),
      timer_);
  });
}

//====================================================
// File level

void
EventProcessor::openInputFile()
{
  actReg_.sPreOpenFile.invoke();
  FDEBUG(1) << spaces(8) << "openInputFile\n";
  fb_ = input_->readFile();
  if (!fb_) {
    throw Exception(errors::LogicError)
      << "Source readFile() did not return a valid FileBlock: FileBlock "
      << "should be valid or readFile() should throw.\n";
  }
  actReg_.sPostOpenFile.invoke(fb_->fileName());
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
  endPathExecutor_->incrementInputFileNumber();
  // Output-file closing on input-file boundaries are tricky since
  // input files must outlive the output files, which often have data
  // copied forward from the input files.  That's why the
  // recordOutputClosureRequests call is made here instead of in a
  // specialization of recordOutputModuleClosureRequests<>.
  endPathExecutor_->recordOutputClosureRequests(Granularity::InputFile);
  if (endPathExecutor_->outputsToClose()) {
    closeSomeOutputFiles();
  }
  respondToCloseInputFile();
  actReg_.sPreCloseFile.invoke();
  input_->closeFile();
  actReg_.sPostCloseFile.invoke();
  FDEBUG(1) << spaces(8) << "closeInputFile\n";
}

void
EventProcessor::openAllOutputFiles()
{
  endPathExecutor_->openAllOutputFiles(*fb_);
  FDEBUG(1) << spaces(8) << "openAllOutputFiles\n";
}

void
EventProcessor::closeAllOutputFiles()
{
  if (!endPathExecutor_->someOutputsOpen()) {
    return;
  }
  respondToCloseOutputFiles();
  endPathExecutor_->closeAllOutputFiles();
  FDEBUG(1) << spaces(8) << "closeAllOutputFiles\n";
}

void
EventProcessor::openSomeOutputFiles()
{
  if (!endPathExecutor_->outputsToOpen()) {
    return;
  }
  endPathExecutor_->openSomeOutputFiles(*fb_);
  FDEBUG(1) << spaces(8) << "openSomeOutputFiles\n";
  respondToOpenOutputFiles();
}

void
EventProcessor::setOutputFileStatus(OutputFileStatus const ofs)
{
  endPathExecutor_->setOutputFileStatus(ofs);
  FDEBUG(1) << spaces(8) << "setOutputFileStatus\n";
}

void
EventProcessor::closeSomeOutputFiles()
{
  // Precondition: there are SOME output files that have been
  //               flagged as needing to close.  Otherwise,
  //               'respondtoCloseOutputFiles' will be needlessly
  //               called.
  assert(endPathExecutor_->outputsToClose());
  respondToCloseOutputFiles();
  endPathExecutor_->closeSomeOutputFiles();
  FDEBUG(1) << spaces(8) << "closeSomeOutputFiles\n";
}

void
EventProcessor::respondToOpenInputFile()
{
  schedule_[ScheduleID::first()].respondToOpenInputFile(*fb_);
  endPathExecutor_->respondToOpenInputFile(*fb_);
  FDEBUG(1) << spaces(8) << "respondToOpenInputFile\n";
}

void
EventProcessor::respondToCloseInputFile()
{
  schedule_[ScheduleID::first()].respondToCloseInputFile(*fb_);
  endPathExecutor_->respondToCloseInputFile(*fb_);
  FDEBUG(1) << spaces(8) << "respondToCloseInputFile\n";
}

void
EventProcessor::respondToOpenOutputFiles()
{
  schedule_[ScheduleID::first()].respondToOpenOutputFiles(*fb_);
  endPathExecutor_->respondToOpenOutputFiles(*fb_);
  FDEBUG(1) << spaces(8) << "respondToOpenOutputFiles\n";
}

void
EventProcessor::respondToCloseOutputFiles()
{
  schedule_[ScheduleID::first()].respondToCloseOutputFiles(*fb_);
  endPathExecutor_->respondToCloseOutputFiles(*fb_);
  FDEBUG(1) << spaces(8) << "respondToCloseOutputFiles\n";
}

//=============================================
// Run level

void
EventProcessor::readRun()
{
  {
    actReg_.sPreSourceRun.invoke();
    runPrincipal_ = input_->readRun();
    // Seeding the RangeSet is necessary here in case
    // 'sPostReadRun.invoke()' throws.
    endPathExecutor_->seedRunRangeSet(input_->runRangeSetHandler());
    assert(runPrincipal_);
    runPrincipal_->enableProductCreation(producedProducts_);
    psSignals_.sPostReadRun.invoke(*runPrincipal_);
  }
  runPrincipal_->setProducedProducts(producedProducts_);
  Run const r{*runPrincipal_, ModuleDescription{}, TypeLabelLookup_t{}};
  actReg_.sPostSourceRun.invoke(r);
  FDEBUG(1) << spaces(8) << "readRun.....................("
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
      Run const run{*runPrincipal_, ModuleDescription{}, TypeLabelLookup_t{}};
      actReg_.sPreBeginRun.invoke(run);
    }
    schedule_[ScheduleID::first()].process(Transition::BeginRun,
                                           *runPrincipal_);
    endPathExecutor_->process(Transition::BeginRun, *runPrincipal_);
    {
      Run const run{*runPrincipal_, ModuleDescription{}, TypeLabelLookup_t{}};
      actReg_.sPostBeginRun.invoke(run);
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
  FDEBUG(1) << spaces(8) << "beginRun....................(" << r << ")\n";
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
  endPathExecutor_->setAuxiliaryRangeSetID(*runPrincipal_);
  FDEBUG(1) << spaces(8) << "setRunAuxiliaryRangeSetID...("
            << runPrincipal_->runID() << ")\n";
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
    {
      actReg_.sPreEndRun.invoke(runPrincipal_->runID(),
                                runPrincipal_->endTime());
    }
    schedule_[ScheduleID::first()].process(Transition::EndRun, *runPrincipal_);
    endPathExecutor_->process(Transition::EndRun, *runPrincipal_);
    {
      Run const r{*runPrincipal_, ModuleDescription{}, TypeLabelLookup_t{}};
      actReg_.sPostEndRun.invoke(r);
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
  FDEBUG(1) << spaces(8) << "endRun......................(" << run << ")\n";
  beginRunCalled_ = false;
}

void
EventProcessor::writeRun()
{
  assert(runPrincipal_);
  // Precondition: The RunID does not correspond to a flush ID.
  RunID const r{runPrincipal_->runID()};
  assert(!r.isFlush());
  endPathExecutor_->writeRun(*runPrincipal_);
  FDEBUG(1) << spaces(8) << "writeRun....................(" << r << ")\n";
}

//=============================================
// SubRun level

void
EventProcessor::readSubRun()
{
  {
    actReg_.sPreSourceSubRun.invoke();
    subRunPrincipal_ = input_->readSubRun(runPrincipal_.get());
    // Seeding the RangeSet is necessary here in case
    // 'sPostSubRun.invoke()' throws.
    endPathExecutor_->seedSubRunRangeSet(input_->subRunRangeSetHandler());
    assert(subRunPrincipal_);
    subRunPrincipal_->enableProductCreation(producedProducts_);
    psSignals_.sPostReadSubRun.invoke(*subRunPrincipal_);
  }
  subRunPrincipal_->setProducedProducts(producedProducts_);
  SubRun const sr{*subRunPrincipal_, ModuleDescription{}, TypeLabelLookup_t{}};
  actReg_.sPostSourceSubRun.invoke(sr);
  FDEBUG(1) << spaces(8) << "readSubRun..................("
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
      SubRun const srun{
        *subRunPrincipal_, ModuleDescription{}, TypeLabelLookup_t{}};
      actReg_.sPreBeginSubRun.invoke(srun);
    }
    schedule_[ScheduleID::first()].process(Transition::BeginSubRun,
                                           *subRunPrincipal_);
    endPathExecutor_->process(Transition::BeginSubRun, *subRunPrincipal_);
    {
      SubRun const srun{
        *subRunPrincipal_, ModuleDescription{}, TypeLabelLookup_t{}};
      actReg_.sPostBeginSubRun.invoke(srun);
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
  FDEBUG(1) << spaces(8) << "beginSubRun.................(" << sr << ")\n";
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
  endPathExecutor_->setAuxiliaryRangeSetID(*subRunPrincipal_);
  FDEBUG(1) << spaces(8) << "setSubRunAuxiliaryRangeSetID("
            << subRunPrincipal_->subRunID() << ")\n";
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
    {
      actReg_.sPreEndSubRun.invoke(subRunPrincipal_->subRunID(),
                                   subRunPrincipal_->endTime());
    }
    schedule_[ScheduleID::first()].process(Transition::EndSubRun,
                                           *subRunPrincipal_);
    endPathExecutor_->process(Transition::EndSubRun, *subRunPrincipal_);
    {
      SubRun const srun{
        *subRunPrincipal_, ModuleDescription{}, TypeLabelLookup_t{}};
      actReg_.sPostEndSubRun.invoke(srun);
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
  FDEBUG(1) << spaces(8) << "endSubRun...................(" << sr << ")\n";
  beginSubRunCalled_ = false;
}

void
EventProcessor::writeSubRun()
{
  assert(subRunPrincipal_);
  // Precondition: The SubRunID does not correspond to a flush ID.
  SubRunID const& sr{subRunPrincipal_->subRunID()};
  assert(!sr.isFlush());
  endPathExecutor_->writeSubRun(*subRunPrincipal_);
  FDEBUG(1) << spaces(8) << "writeSubRun.................(" << sr << ")\n";
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
