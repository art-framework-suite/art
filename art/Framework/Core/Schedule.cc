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
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Utilities/CPCSentry.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/Transition.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ReleaseVersion.h"
#include "canvas/Utilities/DebugMacros.h"
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
    actReg_ = nullptr;
    actionTable_ = nullptr;
  }

  Schedule::Schedule(ScheduleID const scheduleID,
                     PathManager& pm,
                     string const& processName,
                     ParameterSet const& proc_pset,
                     ParameterSet const& trig_pset,
                     UpdateOutputCallbacks& outputCallbacks,
                     ProductDescriptions& productsToProduce,
                     ActionTable const& actions,
                     ActivityRegistry const& actReg)
  {
    ostringstream msg;
    msg << "0x" << hex << ((unsigned long)this) << dec;
    TDEBUG_FUNC_SI_MSG(5, "Schedule ctor", scheduleID, msg.str());
    actionTable_ = &actions;
    actReg_ = &actReg;
    triggerPathsInfo_ = &pm.triggerPathsInfo(scheduleID);
    results_inserter_ = nullptr;
    runningWorkerCnt_ = 0;
    if (triggerPathsInfo_.load()->paths().empty()) {
      return;
    }
    results_inserter_ = pm.triggerResultsInserter(scheduleID);
    if (results_inserter_.load() != nullptr) {
      return;
    }
    // Make the trigger results inserter.
    WorkerParams const wp{proc_pset,
                          trig_pset,
                          outputCallbacks,
                          productsToProduce,
                          *actReg_.load(),
                          *actionTable_.load(),
                          processName,
                          scheduleID};
    ModuleDescription md{
      trig_pset.id(),
      "TriggerResultInserter",
      "TriggerResults",
      static_cast<int>(ModuleThreadingType::replicated),
      ProcessConfiguration{processName, proc_pset.id(), getReleaseVersion()}};
    string pathName{"ctor"};
    CurrentProcessingContext cpc{scheduleID, &pathName, 0, false};
    cpc.activate(0, &md);
    detail::CPCSentry cpc_sentry{cpc};
    actReg_.load()->sPreModuleConstruction.invoke(md);
    EDProducer* producer = new TriggerResultInserter(
      trig_pset, scheduleID, triggerPathsInfo_.load()->pathResults());
    producer->setModuleDescription(md);
    producer->setScheduleID(scheduleID);
    pm.setTriggerResultsInserter(
      scheduleID, make_unique<WorkerT<EDProducer>>(producer, md, wp));
    results_inserter_ = pm.triggerResultsInserter(scheduleID);
    actReg_.load()->sPostModuleConstruction.invoke(md);
  }

  Schedule::Schedule(Schedule&& rhs) noexcept
  {
    actionTable_ = rhs.actionTable_.load();
    rhs.actionTable_ = nullptr;
    actReg_ = rhs.actReg_.load();
    rhs.actReg_ = nullptr;
    triggerPathsInfo_ = rhs.triggerPathsInfo_.load();
    rhs.triggerPathsInfo_ = nullptr;
    results_inserter_ = rhs.results_inserter_.load();
    rhs.results_inserter_ = nullptr;
    runningWorkerCnt_ = rhs.runningWorkerCnt_.load();
  }

  Schedule&
  Schedule::operator=(Schedule&& rhs) noexcept
  {
    actionTable_ = rhs.actionTable_.load();
    rhs.actionTable_ = nullptr;
    actReg_ = rhs.actReg_.load();
    rhs.actReg_ = nullptr;
    triggerPathsInfo_ = rhs.triggerPathsInfo_.load();
    rhs.triggerPathsInfo_ = nullptr;
    results_inserter_ = rhs.results_inserter_.load();
    rhs.results_inserter_ = nullptr;
    runningWorkerCnt_ = rhs.runningWorkerCnt_.load();
    return *this;
  }

  void
  Schedule::beginJob()
  {
    for (auto const& val : triggerPathsInfo_.load()->workers()) {
      auto& w = *val.second;
      w.beginJob();
    }
    if (results_inserter_.load() != nullptr) {
      results_inserter_.load()->beginJob();
    }
  }

  void
  Schedule::endJob()
  {
    Exception error(errors::EndJobFailure);
    for (auto& val : triggerPathsInfo_.load()->workers()) {
      auto& w = *val.second;
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
    if (results_inserter_.load() != nullptr) {
      // FIXME: The catch and rethrow here seems to have little value added.
      try {
        results_inserter_.load()->endJob();
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
      w.respondToOpenInputFile(fb);
    }
    if (results_inserter_.load() != nullptr) {
      results_inserter_.load()->respondToOpenInputFile(fb);
    }
  }

  void
  Schedule::respondToCloseInputFile(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.load()->workers()) {
      auto& w = *val.second;
      w.respondToCloseInputFile(fb);
    }
    if (results_inserter_.load() != nullptr) {
      results_inserter_.load()->respondToCloseInputFile(fb);
    }
  }

  void
  Schedule::respondToOpenOutputFiles(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.load()->workers()) {
      auto& w = *val.second;
      w.respondToOpenOutputFiles(fb);
    }
    if (results_inserter_.load() != nullptr) {
      results_inserter_.load()->respondToOpenOutputFiles(fb);
    }
  }

  void
  Schedule::respondToCloseOutputFiles(FileBlock const& fb)
  {
    for (auto const& val : triggerPathsInfo_.load()->workers()) {
      auto& w = *val.second;
      w.respondToCloseOutputFiles(fb);
    }
    if (results_inserter_.load() != nullptr) {
      results_inserter_.load()->respondToCloseOutputFiles(fb);
    }
  }

  void
  Schedule::process(Transition const trans, Principal& principal)
  {
    for (auto const& val : triggerPathsInfo_.load()->workers()) {
      val.second->reset(ScheduleID::first());
    }
    for (auto const& path : triggerPathsInfo_.load()->paths()) {
      path->process(trans, principal);
    }
  }

  class PathsDoneFunctor {
  public:
    PathsDoneFunctor(Schedule* schedule,
                     WaitingTask* endPathTask,
                     tbb::task* eventLoopTask,
                     EventPrincipal& principal,
                     ScheduleID const scheduleID)
      : schedule_(schedule)
      , endPathTask_(endPathTask)
      , eventLoopTask_(eventLoopTask)
      , principal_(principal)
      , si_(scheduleID)
    {}
    void
    operator()(exception_ptr const* ex)
    {
      schedule_->pathsDoneTask(
        endPathTask_, eventLoopTask_, principal_, si_, ex);
    }

  private:
    Schedule* schedule_;
    WaitingTask* endPathTask_;
    tbb::task* eventLoopTask_;
    EventPrincipal& principal_;
    ScheduleID const si_;
  };

  void
  Schedule::pathsDoneTask(WaitingTask* endPathTask,
                          tbb::task* eventLoopTask,
                          EventPrincipal& principal,
                          ScheduleID const scheduleID,
                          exception_ptr const* ex)
  {
    // Note: When we start our parent task is the eventLoop task.
    TDEBUG_BEGIN_TASK_SI(4, "pathsDoneTask", scheduleID);
    INTENTIONAL_DATA_RACE(DR_SCHEDULE_PATHS_DONE_TASK);
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
          TDEBUG_END_TASK_SI_ERR(
            4,
            "pathsDoneTask",
            scheduleID,
            "trigger path processing terminate because of EXCEPTION");
          return;
        }
        // FIXME: We should not ever be able to get here because the only
        // exceptions passed to the pathsDone task should be ones that
        // terminated the path.
        mf::LogWarning(e.category())
          << "an exception occurred and all paths for the event "
             "are being skipped: \n"
          << cet::trim_right_copy(e.what(), " \n");
      }
      // FIXME: We should not ever be able to get here because the only
      // exceptions passed to the pathsDone task should be ones that terminated
      // the path. Transfer the thrown exception to the endPath task and start
      // it running.
      WaitingTaskHolder wth(endPathTask);
      wth.doneWaiting(*ex);
      // And end this task without terminating
      // event processing.
      ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                   sizeof(tbb::internal::task_prefix),
                                 sizeof(tbb::task) +
                                   sizeof(tbb::internal::task_prefix),
                                 "tbb::task");
      tbb::task::self().set_parent(nullptr);
      TDEBUG_END_TASK_SI_ERR(
        4,
        "pathsDoneTask",
        scheduleID,
        "trigger path processing terminate because of EXCEPTION");
      return;
    }
    process_event_pathsDone(endPathTask, eventLoopTask, principal, scheduleID);
    // And end this task, which does not terminate
    // event processing because our parent is the
    // nullptr.
    TDEBUG_END_TASK_SI(4, "pathsDoneTask", scheduleID);
    return;
  }

  // Note: We get here as part of the readAndProcessEvent task.
  // Our parent task is the nullptr, and the parent task of the
  // endPathTask is the eventLoopTask.
  void
  Schedule::process_event(WaitingTask* endPathTask,
                          tbb::task* eventLoopTask,
                          EventPrincipal& principal,
                          ScheduleID const scheduleID)
  {
    // Note: We are part of the readAndProcessEventTask (stream head task),
    // and our parent task is the nullptr because the endPathTask has
    // been transferred the eventLoopTask as its parent.
    TDEBUG_BEGIN_FUNC_SI(4, "Schedule::process_event", scheduleID);
    if (runningWorkerCnt_.load() != 0) {
      cerr << "Aborting! runningWorkerCnt_.load() != 0: "
           << runningWorkerCnt_.load() << "\n";
      abort();
    }
    ++runningWorkerCnt_;
    for (auto const& val : triggerPathsInfo_.load()->workers()) {
      auto& w = *val.second;
      w.reset(scheduleID);
    }
    if (results_inserter_.load() != nullptr) {
      results_inserter_.load()->reset(scheduleID);
    }
    triggerPathsInfo_.load()->pathResults().reset();
    triggerPathsInfo_.load()->incrementTotalEventCount();
    auto pathsDoneTask = make_waiting_task(
      tbb::task::allocate_root(),
      PathsDoneFunctor{
        this, endPathTask, eventLoopTask, principal, scheduleID});
    // Allow the pathsDoneTask to terminate event processing on error.
    // FIXME: Actually it turns out it never wants to.  :-)
    ANNOTATE_BENIGN_RACE_SIZED(reinterpret_cast<char*>(&tbb::task::self()) -
                                 sizeof(tbb::internal::task_prefix),
                               sizeof(tbb::task) +
                                 sizeof(tbb::internal::task_prefix),
                               "tbb::task");
    pathsDoneTask->set_parent(eventLoopTask);
    try {
      // Note: We create the holder here to increment the ref count
      // on the pathsDoneTask so that if a path errors quickly and
      // decrements the ref count (using doneWaiting) the task will
      // not run until we have actually started all the tasks.
      // Note: This is critically dependent on the path incrementing
      // the ref count the first thing it does (by putting the task
      // into a WaitingTaskList).
      WaitingTaskHolder wth(pathsDoneTask);
      for (auto& path : triggerPathsInfo_.load()->paths()) {
        // Start each path running.  The path will start a spawn
        // chain going to run each worker in the order specified
        // on the path, and when they have all been run, it will
        // call doneWaiting() on the pathsDoneTask, which decrements
        // its reference count, which will eventually cause it to
        // run when every path has finished.
        INTENTIONAL_DATA_RACE(DR_SCHEDULE_PROCESS_EVENT_PER_PATH);
        path->process_event(pathsDoneTask, principal, scheduleID);
      }
      // And end this task which does not terminate event processing
      // because our parent is the nullptr.
      --runningWorkerCnt_;
      TDEBUG_END_FUNC_SI(4, "Schedule::process_event", scheduleID);
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
        TDEBUG_END_FUNC_SI_ERR(
          4, "Schedule::process_event", scheduleID, "because of EXCEPTION");
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
    TDEBUG_END_FUNC_SI(4, "Schedule::process_event", scheduleID);
  }

  // Note: We come here as part of the pathsDone task.  Our parent
  // is the nullptr.
  void
  Schedule::process_event_pathsDone(WaitingTask* endPathTask,
                                    WaitingTask* /*eventLoopTask*/,
                                    EventPrincipal& principal,
                                    ScheduleID const scheduleID)
  {
    // We are part of the pathsDoneTask, and our parent is the nullptr.
    TDEBUG_BEGIN_FUNC_SI(4, "Schedule::process_event_pathsDone", scheduleID);
    try {
      if (triggerPathsInfo_.load()->pathResults().accept()) {
        triggerPathsInfo_.load()->incrementPassedEventCount();
      }
      if (results_inserter_.load() != nullptr) {
        string const name{"TriggerResultsInserter"};
        CurrentProcessingContext cpc{scheduleID, &name, 0, false};
        results_inserter_.load()->doWork_event(principal, scheduleID, &cpc);
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
        TDEBUG_END_FUNC_SI(4, "Schedule::process_event_pathsDone", scheduleID);
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
    TDEBUG_END_FUNC_SI(4, "Schedule::process_event_pathsDone", scheduleID);
  }

} // namespace art
