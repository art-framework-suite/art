#include "art/Framework/Core/Schedule.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/OutputModuleDescription.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Core/TriggerReport.h"
#include "art/Framework/Core/TriggerResultInserter.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/skip_non_replicated.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/PathContext.h"
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/TaskDebugMacros.h"
#include "art/Utilities/Transition.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Provenance/ReleaseVersion.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskHolder.h"
#include "hep_concurrency/tsan.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

using namespace hep::concurrency;
using namespace std;

using fhicl::ParameterSet;

namespace art {

  Schedule::~Schedule() noexcept
  {
    results_inserter_ = nullptr;
    triggerPathsInfo_ = nullptr;
    actionTable_ = nullptr;
  }

  Schedule::Schedule(ScheduleID const scheduleID,
                     PathManager& pm,
                     ActionTable const& actions,
                     std::unique_ptr<Worker> triggerResultsInserter)
    : sc_{scheduleID}
  {
    TDEBUG_FUNC_SI(5, scheduleID) << hex << this << dec;
    actionTable_ = &actions;
    triggerPathsInfo_ = &pm.triggerPathsInfo(scheduleID);
    results_inserter_ = std::move(triggerResultsInserter);
    runningWorkerCnt_ = 0;
  }

  void
  Schedule::beginJob()
  {
    for (auto const& val : triggerPathsInfo_.load()->workers()) {
      auto& w = *val.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.beginJob();
    }
    if (results_inserter_) {
      results_inserter_->beginJob();
    }
  }

  void
  Schedule::endJob()
  {
    Exception error{errors::EndJobFailure};
    for (auto& val : triggerPathsInfo_.load()->workers()) {
      auto& w = *val.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      // FIXME: The catch and rethrow here seems to have little value added.
      try {
        w.endJob();
      }
      catch (cet::exception& e) {
        error << "cet::exception caught in Schedule::endJob\n"
              << e.explain_self();
        throw error;
      }
      catch (exception& e) {
        error << "Standard library exception caught in Schedule::endJob\n"
              << e.what();
        throw error;
      }
      catch (...) {
        error << "Unknown exception caught in Schedule::endJob\n";
        throw error;
      }
    }
    if (results_inserter_) {
      // FIXME: The catch and rethrow here seems to have little value added.
      try {
        results_inserter_->endJob();
      }
      catch (cet::exception& e) {
        error << "cet::exception caught in Schedule::endJob\n"
              << e.explain_self();
        throw error;
      }
      catch (exception& e) {
        error << "Standard library exception caught in Schedule::endJob\n"
              << e.what();
        throw error;
      }
      catch (...) {
        error << "Unknown exception caught in Schedule::endJob\n";
        throw error;
      }
    }
  }

  void
  Schedule::respondToOpenInputFile(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.load()->workers()) {
      auto& w = *val.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.respondToOpenInputFile(fb);
    }
    if (results_inserter_) {
      results_inserter_->respondToOpenInputFile(fb);
    }
  }

  void
  Schedule::respondToCloseInputFile(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.load()->workers()) {
      auto& w = *val.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.respondToCloseInputFile(fb);
    }
    if (results_inserter_) {
      results_inserter_->respondToCloseInputFile(fb);
    }
  }

  void
  Schedule::respondToOpenOutputFiles(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.load()->workers()) {
      auto& w = *val.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.respondToOpenOutputFiles(fb);
    }
    if (results_inserter_) {
      results_inserter_->respondToOpenOutputFiles(fb);
    }
  }

  void
  Schedule::respondToCloseOutputFiles(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.load()->workers()) {
      auto& w = *val.second;
      if (detail::skip_non_replicated(w)) {
        continue;
      }
      w.respondToCloseOutputFiles(fb);
    }
    if (results_inserter_) {
      results_inserter_->respondToCloseOutputFiles(fb);
    }
  }

  void
  Schedule::process(Transition const trans, Principal& principal)
  {
    for (auto const& val : triggerPathsInfo_.load()->workers()) {
      val.second->reset();
    }
    for (auto const& path : triggerPathsInfo_.load()->paths()) {
      path->process(trans, principal);
    }
  }

  class PathsDoneFunctor {
  public:
    PathsDoneFunctor(Schedule* const schedule,
                     WaitingTask* const endPathTask,
                     tbb::task* const eventLoopTask,
                     EventPrincipal& principal)
      : schedule_{schedule}
      , endPathTask_{endPathTask}
      , eventLoopTask_{eventLoopTask}
      , principal_{principal}
    {}
    void
    operator()(exception_ptr const* ex)
    {
      schedule_->pathsDoneTask(endPathTask_, eventLoopTask_, principal_, ex);
    }

  private:
    Schedule* const schedule_;
    WaitingTask* const endPathTask_;
    tbb::task* const eventLoopTask_;
    EventPrincipal& principal_;
  };

  void
  Schedule::pathsDoneTask(WaitingTask* endPathTask,
                          tbb::task* eventLoopTask,
                          EventPrincipal& principal,
                          exception_ptr const* ex)
  {
    auto const scheduleID = sc_.id();
    // Note: When we start our parent task is the eventLoop task.
    TDEBUG_BEGIN_TASK_SI(4, scheduleID);
    if (ex != nullptr) {
      try {
        rethrow_exception(*ex);
      }
      catch (cet::exception& e) {
        auto action = actionTable_.load()->find(e.root_cause());
        assert(action != actions::IgnoreCompletely);
        assert(action != actions::FailPath);
        assert(action != actions::FailModule);
        if (action != actions::SkipEvent) {
          WaitingTaskHolder wth(endPathTask);
          wth.doneWaiting(current_exception());
          // And end this task which does not terminate event processing.
          ANNOTATE_BENIGN_RACE_SIZED(
            reinterpret_cast<char*>(&tbb::task::self()) -
              sizeof(tbb::internal::task_prefix),
            sizeof(tbb::task) + sizeof(tbb::internal::task_prefix),
            "tbb::task");
          tbb::task::self().set_parent(nullptr);
          TDEBUG_END_TASK_SI(4, scheduleID)
            << "trigger path processing terminate because of EXCEPTION";
          return;
        }
        // FIXME: We should not ever be able to get here because the
        // only exceptions passed to the pathsDone task should be ones
        // that terminated the path.
        mf::LogWarning(e.category())
          << "an exception occurred and all paths for the event "
             "are being skipped: \n"
          << cet::trim_right_copy(e.what(), " \n");
      }
      // FIXME: We should not ever be able to get here because the
      // only exceptions passed to the pathsDone task should be ones
      // that terminated the path. Transfer the thrown exception to
      // the endPath task and start it running.
      WaitingTaskHolder wth(endPathTask);
      wth.doneWaiting(*ex);
      // And end this task without terminating event processing.
      ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                   sizeof(tbb::internal::task_prefix),
                                 sizeof(tbb::task) +
                                   sizeof(tbb::internal::task_prefix),
                                 "tbb::task");
      tbb::task::self().set_parent(nullptr);
      TDEBUG_END_TASK_SI(4, scheduleID)
        << "trigger path processing terminate because of EXCEPTION";
      return;
    }
    process_event_pathsDone(endPathTask, eventLoopTask, principal);
    // And end this task, which does not terminate event processing
    // because our parent is the nullptr.
    TDEBUG_END_TASK_SI(4, scheduleID);
  }

  // Note: We get here as part of the readAndProcessEvent task.  Our
  // parent task is the nullptr, and the parent task of the
  // endPathTask is the eventLoopTask.
  void
  Schedule::process_event(WaitingTask* endPathTask,
                          tbb::task* eventLoopTask,
                          EventPrincipal& principal)
  {
    auto const scheduleID = sc_.id();
    // Note: We are part of the readAndProcessEventTask (stream head
    // task), and our parent task is the nullptr because the
    // endPathTask has been transferred the eventLoopTask as its
    // parent.
    TDEBUG_BEGIN_FUNC_SI(4, scheduleID);
    if (runningWorkerCnt_.load() != 0) {
      cerr << "Aborting! runningWorkerCnt_.load() != 0: "
           << runningWorkerCnt_.load() << "\n";
      abort();
    }
    ++runningWorkerCnt_;
    for (auto const& val : triggerPathsInfo_.load()->workers()) {
      auto& w = *val.second;
      w.reset();
    }
    if (results_inserter_) {
      results_inserter_->reset();
    }
    triggerPathsInfo_.load()->pathResults().reset();
    triggerPathsInfo_.load()->incrementTotalEventCount();
    auto pathsDoneTask = make_waiting_task(
      tbb::task::allocate_root(),
      PathsDoneFunctor{this, endPathTask, eventLoopTask, principal});
    // Allow the pathsDoneTask to terminate event processing on error.
    // FIXME: Actually it turns out it never wants to.  :-)
    ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                 sizeof(tbb::internal::task_prefix),
                               sizeof(tbb::task) +
                                 sizeof(tbb::internal::task_prefix),
                               "tbb::task");
    pathsDoneTask->set_parent(eventLoopTask);
    try {
      // Note: We create the holder here to increment the ref count on
      // the pathsDoneTask so that if a path errors quickly and
      // decrements the ref count (using doneWaiting) the task will
      // not run until we have actually started all the tasks.  Note:
      // This is critically dependent on the path incrementing the ref
      // count the first thing it does (by putting the task into a
      // WaitingTaskList).
      WaitingTaskHolder wth(pathsDoneTask);
      for (auto& path : triggerPathsInfo_.load()->paths()) {
        // Start each path running.  The path will start a spawn chain
        // going to run each worker in the order specified on the
        // path, and when they have all been run, it will call
        // doneWaiting() on the pathsDoneTask, which decrements its
        // reference count, which will eventually cause it to run when
        // every path has finished.
        path->process_event(pathsDoneTask, principal);
      }
      // And end this task which does not terminate event processing
      // because our parent is the nullptr.
      --runningWorkerCnt_;
      TDEBUG_END_FUNC_SI(4, scheduleID);
      return;
    }
    catch (cet::exception& e) {
      auto action = actionTable_.load()->find(e.root_cause());
      assert(action != actions::IgnoreCompletely);
      assert(action != actions::FailPath);
      assert(action != actions::FailModule);
      if (action != actions::SkipEvent) {
        WaitingTaskHolder wth(endPathTask);
        wth.doneWaiting(current_exception());
        // And end this task which does not terminate event processing
        // because our parent is the nullptr.
        --runningWorkerCnt_;
        TDEBUG_END_FUNC_SI(4, scheduleID) << "because of EXCEPTION";
        return;
      }
      mf::LogWarning(e.category())
        << "an exception occurred and all paths for the event "
           "are being skipped: \n"
        << cet::trim_right_copy(e.what(), " \n");
    }
    WaitingTaskHolder wth(endPathTask);
    wth.doneWaiting(exception_ptr{});
    // And end this task which does not terminate event processing
    // because our parent is the nullptr.
    --runningWorkerCnt_;
    TDEBUG_END_FUNC_SI(4, scheduleID);
  }

  // Note: We come here as part of the pathsDone task.  Our parent is
  // the nullptr.
  void
  Schedule::process_event_pathsDone(WaitingTask* endPathTask,
                                    WaitingTask* /*eventLoopTask*/,
                                    EventPrincipal& principal)
  {
    auto const scheduleID = sc_.id();
    // We are part of the pathsDoneTask, and our parent is the nullptr.
    TDEBUG_BEGIN_FUNC_SI(4, scheduleID);
    try {
      if (triggerPathsInfo_.load()->pathResults().accept()) {
        triggerPathsInfo_.load()->incrementPassedEventCount();
      }
      if (results_inserter_) {
        // FIXME: not sure what the trigger bit should be
        auto const& resultsInserterDesc = results_inserter_->description();
        PathContext const pc{sc_,
                             PathContext::art_path(),
                             -1,
                             {resultsInserterDesc.moduleLabel()}};
        ModuleContext const mc{pc, resultsInserterDesc};
        results_inserter_->doWork_event(principal, mc);
      }
    }
    catch (cet::exception& e) {
      auto action = actionTable_.load()->find(e.root_cause());
      assert(action != actions::IgnoreCompletely);
      assert(action != actions::FailPath);
      assert(action != actions::FailModule);
      if (action != actions::SkipEvent) {
        // Possible actions: Rethrow
        // FIXME: Do a doneWaiting on the endPathTask instead!
        TDEBUG_END_FUNC_SI(4, scheduleID);
        throw;
      }
      mf::LogWarning(e.category())
        << "An exception occurred inserting the TriggerResults object:\n"
        << cet::trim_right_copy(e.what(), " \n");
    }
    // And end this task without terminating event processing.
    ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                 sizeof(tbb::internal::task_prefix),
                               sizeof(tbb::task) +
                                 sizeof(tbb::internal::task_prefix),
                               "tbb::task");
    tbb::task::self().set_parent(nullptr);
    // Start the endPathTask going.
    WaitingTaskHolder wth(endPathTask);
    wth.doneWaiting(exception_ptr{});
    TDEBUG_END_FUNC_SI(4, scheduleID);
  }
} // namespace art
