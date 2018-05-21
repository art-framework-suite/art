#include "art/Framework/Core/Path.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "canvas/Persistency/Common/HLTPathStatus.h"
#include "canvas/Persistency/Common/HLTenums.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "canvas/Utilities/DebugMacros.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskHolder.h"
#include "hep_concurrency/WaitingTaskList.h"
#include "hep_concurrency/tsan.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "tbb/task.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <exception>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace cet;
using namespace fhicl;
using namespace hep::concurrency;
using namespace std;

namespace art {

  class EventPrincipal;

  Path::~Path()
  {
    actionTable_ = nullptr;
    actReg_ = nullptr;
    delete workers_.load();
    workers_ = nullptr;
    delete waitingTasks_.load();
    waitingTasks_ = nullptr;
  }

  Path::Path(ActionTable const& actions,
             ActivityRegistry const& actReg,
             PathContext const& pc,
             vector<WorkerInPath>&& workers,
             HLTGlobalStatus* pathResults) noexcept
    : pc_{pc}, bitpos_{pc.bitPosition()}
  {
    {
      ostringstream msg;
      msg << "0x" << hex << ((unsigned long)this) << dec;
      TDEBUG_FUNC_SI_MSG(4, "Path ctor", pc_.scheduleID(), msg.str());
    }
    actionTable_ = &actions;
    actReg_ = &actReg;
    workers_ = new vector<WorkerInPath>{move(workers)};
    trptr_ = pathResults;
    waitingTasks_ = new WaitingTaskList;
    state_ = hlt::Ready;
    timesRun_ = 0;
    timesPassed_ = 0;
    timesFailed_ = 0;
    timesExcept_ = 0;
  }

  ScheduleID
  Path::scheduleID() const
  {
    return pc_.scheduleID();
  }

  int
  Path::bitPosition() const
  {
    return bitpos_;
  }

  string const&
  Path::name() const
  {
    return pc_.pathName();
  }

  size_t
  Path::timesRun() const
  {
    return timesRun_;
  }

  size_t
  Path::timesPassed() const
  {
    return timesPassed_;
  }

  size_t
  Path::timesFailed() const
  {
    return timesFailed_;
  }

  size_t
  Path::timesExcept() const
  {
    return timesExcept_;
  }

  hlt::HLTState
  Path::state() const
  {
    return state_;
  }

  vector<WorkerInPath> const&
  Path::workersInPath() const
  {
    return *workers_.load();
  }

  void
  Path::clearCounters()
  {
    timesRun_ = 0;
    timesPassed_ = 0;
    timesFailed_ = 0;
    timesExcept_ = 0;
    for (auto& w : *workers_.load()) {
      w.clearCounters();
    }
  }

  void
  Path::process(Transition const trans, Principal& principal)
  {
    if (trans == Transition::BeginRun) {
      actReg_.load()->sPrePathBeginRun.invoke(name());
    } else if (trans == Transition::EndRun) {
      actReg_.load()->sPrePathEndRun.invoke(name());
    } else if (trans == Transition::BeginSubRun) {
      actReg_.load()->sPrePathBeginSubRun.invoke(name());
    } else if (trans == Transition::EndSubRun) {
      actReg_.load()->sPrePathEndSubRun.invoke(name());
    }
    state_ = hlt::Ready;
    bool should_continue = true;
    std::size_t idx = 0;
    for (auto I = workers_.load()->begin(), E = workers_.load()->end();
         (I != E) && should_continue;
         ++I, ++idx) {
      try {
        should_continue = I->runWorker(trans, principal);
      }
      catch (cet::exception& e) {
        state_ = hlt::Exception;
        throw art::Exception{
          errors::ScheduleExecutionFailure, "Path: ProcessingStopped.", e}
          << "Exception going through path " << name() << "\n";
      }
      catch (...) {
        mf::LogError("PassingThrough")
          << "Exception passing through path " << name() << "\n";
        state_ = hlt::Exception;
        throw;
      }
    }
    if (should_continue) {
      state_ = hlt::Pass;
    } else {
      state_ = hlt::Fail;
    }
    HLTPathStatus const status(state_, idx);
    if (trans == Transition::BeginRun) {
      actReg_.load()->sPostPathBeginRun.invoke(name(), status);
    } else if (trans == Transition::EndRun) {
      actReg_.load()->sPostPathEndRun.invoke(name(), status);
    } else if (trans == Transition::BeginSubRun) {
      actReg_.load()->sPostPathBeginSubRun.invoke(name(), status);
    } else if (trans == Transition::EndSubRun) {
      actReg_.load()->sPostPathEndSubRun.invoke(name(), status);
    }
  }

  // We come here as part of the endPath task.  Our parent is the
  // eventLoop task.  We will run the workers on the path in order
  // serially without using tasks.
  void
  Path::process_event_for_endpath(EventPrincipal& ep)
  {
    auto const sid = pc_.scheduleID();
    TDEBUG_BEGIN_FUNC_SI(4, "Path::process_event_for_endpath", sid);
    actReg_.load()->sPreProcessPath.invoke(pc_);
    ++timesRun_;
    state_ = hlt::Ready;
    auto const max_idx = workers_.load()->size();
    size_t idx = 0;
    bool should_continue = true;
    for (; should_continue && (idx < max_idx); ++idx) {
      auto& workerInPath = (*workers_.load())[idx];
      try {
        workerInPath.runWorker_event_for_endpath(ep);
      }
      catch (cet::exception& e) {
        // Possible actions: IgnoreCompletely, Rethrow, SkipEvent, FailModule,
        // FailPath
        auto action = actionTable_.load()->find(e.root_cause());
        assert(action != actions::FailModule);
        if (action != actions::FailPath) {
          // Possible actions: IgnoreCompletely, Rethrow, SkipEvent
          ++timesExcept_;
          state_ = hlt::Exception;
          if (trptr_.load()) {
            // Not the end path (no trigger results for end path!).
            (*trptr_.load())[bitpos_] = HLTPathStatus(state_, idx);
          }
          TDEBUG_END_FUNC_SI_ERR(4,
                                 "Path::process_event_for_endpath",
                                 sid,
                                 "terminate path because of EXCEPTION");
          throw Exception{
            errors::ScheduleExecutionFailure, "Path: ProcessingStopped.", e}
            << "Exception going through path " << name() << "\n";
        }
        // Possible actions: FailPath
        should_continue = false;
        mf::LogWarning(e.category())
          << "Failing path " << name() << ", due to exception, message:\n"
          << e.what() << "\n";
        // WARNING: We continue processing below!!! The only way we can get here
        // is if the worker threw and we are ignoring the exception but failing
        // the path because of actions::FailPath!!!
      }
      catch (...) {
        mf::LogError("PassingThrough")
          << "Exception passing through path " << name() << "\n";
        ++timesExcept_;
        state_ = hlt::Exception;
        if (trptr_.load()) {
          // Not the end path (no trigger results for end path!).
          (*trptr_.load())[bitpos_] = HLTPathStatus(state_, idx);
        }
        TDEBUG_END_FUNC_SI_ERR(4,
                               "Path::process_event_for_endpath",
                               sid,
                               "terminate end path because of EXCEPTION");
        return;
      }
      // Note: This will only be set false by a filter which has rejected
      // (impossible on the end path!).
      should_continue = workerInPath.returnCode();
      {
        ostringstream msg;
        msg << "idx: " << idx << " should_continue: " << should_continue;
        TDEBUG_FUNC_SI_MSG(
          5, "Path::process_event_for_endpath", sid, msg.str());
      }
    }
    // All done, or filter rejected, or error.
    try {
      if (should_continue) {
        ++timesPassed_;
        state_ = hlt::Pass;
      } else {
        ++timesFailed_;
        state_ = hlt::Fail;
      }
      if (trptr_.load()) {
        // Not the end path.
        (*trptr_.load())[bitpos_] = HLTPathStatus(state_, idx);
      }
      HLTPathStatus const status(state_, idx);
      actReg_.load()->sPostProcessPath.invoke(pc_, status);
    }
    catch (...) {
      TDEBUG_END_FUNC_SI_ERR(
        4,
        "Path::process_event_for_endpath",
        sid,
        "terminate end path final processing because of EXCEPTION");
      throw;
    }
    TDEBUG_END_FUNC_SI(4, "Path::process_event_for_endpath", sid);
  }

  // We come here as part of the readAndProcessEvent task, or as part
  // of the endPath task.  Our parent is the nullptr.  The parent of
  // the pathsDoneTask is the eventLoop task.
  void
  Path::process_event(WaitingTask* pathsDoneTask, EventPrincipal& ep)
  {
    auto const sid = pc_.scheduleID();
    // Note: We are part of the readAndProcessEventTask (stream head task),
    // or the endPath task, and our parent task is the nullptr.
    // The parent of the pathsDoneTask is the eventLoop Task.
    TDEBUG_BEGIN_FUNC_SI(4, "Path::process_event", sid);
    {
      ostringstream msg;
      msg << "0x" << hex << ((unsigned long)this) << dec
          << " Resetting waitingTasks_";
      TDEBUG_FUNC_SI_MSG(6, "Path::process_event", sid, msg.str());
    }

    // Make sure the list is not auto-spawning tasks.
    waitingTasks_.load()->reset();
    // Note: This task list will never have more than one entry.
    waitingTasks_.load()->add(pathsDoneTask);
    {
      actReg_.load()->sPreProcessPath.invoke(pc_);
    }
    ++timesRun_;
    state_ = hlt::Ready;
    size_t idx = 0;
    auto max_idx = workers_.load()->size();
    // Start the task spawn chain going with the first worker on the
    // path.  Each worker will spawn the next worker in order, until
    // all the workers have run.
    process_event_idx_asynch(idx, max_idx, ep);
    TDEBUG_END_FUNC_SI(4, "Path::process_event", sid);
  }

  namespace {
    class RunWorkerFunctor {
    public:
      RunWorkerFunctor(Path* path,
                       size_t const idx,
                       size_t const max_idx,
                       EventPrincipal& ep)
        : path_{path}, idx_{idx}, max_idx_{max_idx}, ep_{ep}
      {}
      void
      operator()(exception_ptr const* ex) const
      {
        path_->runWorkerTask(idx_, max_idx_, ep_, ex);
      }

    private:
      Path* path_;
      size_t const idx_;
      size_t const max_idx_;
      EventPrincipal& ep_;
    };
  }

  void
  Path::runWorkerTask(size_t idx,
                      size_t const max_idx,
                      EventPrincipal& ep,
                      std::exception_ptr const*)
  {
    auto const sid = pc_.scheduleID();
    // Note: When we start here our parent task is the nullptr.
    TDEBUG_BEGIN_TASK_SI(4, "runWorkerTask", sid);
    auto new_idx = idx;
    try {
      process_event_idx(new_idx, max_idx, ep);
    }
    catch (...) {
      waitingTasks_.load()->doneWaiting(current_exception());
      // End this task, terminating the path here.
      TDEBUG_END_TASK_SI_ERR(
        4, "runWorkerTask", sid, "path terminate because of EXCEPTION");
      return;
    }
    TDEBUG_END_TASK_SI(4, "runWorkerTask", sid);
  }

  // This function is a spawn chain system to run workers one at a time,
  // in the order specified on the path, and then decrement the ref count
  // on the endPathsTask when finished (which causes it to run if we are
  // the last path to finish running its workers).
  void
  Path::process_event_idx_asynch(size_t const idx,
                                 size_t const max_idx,
                                 EventPrincipal& ep)
  {
    auto const sid = pc_.scheduleID();
    {
      ostringstream msg;
      msg << "idx: " << idx << " max_idx: " << max_idx;
      TDEBUG_FUNC_SI_MSG(
        4, "Begin Path::process_event_idx_asynch", sid, msg.str());
    }
    auto runWorkerTask = make_waiting_task(
      tbb::task::allocate_root(), RunWorkerFunctor{this, idx, max_idx, ep});
    tbb::task::spawn(*runWorkerTask);
    // And end this task, which does not terminate event
    // processing because our parent task is the nullptr.
    {
      ostringstream msg;
      msg << "idx: " << idx << " max_idx: " << max_idx;
      TDEBUG_FUNC_SI_MSG(4, "Path::process_event_idx_asynch", sid, msg.str());
    }
  }

  namespace {
    class WorkerDoneFunctor {
    public:
      WorkerDoneFunctor(Path* path,
                        size_t const idx,
                        size_t const max_idx,
                        EventPrincipal& ep)
        : path_{path}, idx_{idx}, max_idx_{max_idx}, ep_{ep}
      {}
      void
      operator()(exception_ptr const* ex)
      {
        path_->workerDoneTask(idx_, max_idx_, ep_, ex);
      }

    private:
      Path* path_;
      size_t const idx_;
      size_t const max_idx_;
      EventPrincipal& ep_;
    };
  }

  void
  Path::workerDoneTask(size_t const idx,
                       size_t const max_idx,
                       EventPrincipal& ep,
                       exception_ptr const* ex)
  {
    auto const sid = pc_.scheduleID();
    TDEBUG_BEGIN_TASK_SI(4, "workerDoneTask", sid);
    auto& workerInPath = (*workers_.load())[idx];
    // Note: This will only be set false by a filter which has rejected.
    bool new_should_continue = workerInPath.returnCode();
    {
      ostringstream msg;
      msg << "new_should_continue: " << new_should_continue;
      TDEBUG_TASK_SI_MSG(4, "workerDoneTask", sid, msg.str());
    }
    if (ex != nullptr) {
      try {
        rethrow_exception(*ex);
      }
      catch (cet::exception& e) {
        auto action = actionTable_.load()->find(e.root_cause());
        assert(action != actions::FailModule);
        if (action != actions::FailPath) {
          // Possible actions: IgnoreCompletely, Rethrow, SkipEvent
          ++timesExcept_;
          state_ = hlt::Exception;
          if (trptr_.load()) {
            // Not the end path.
            (*trptr_.load())[bitpos_] = HLTPathStatus(state_, idx);
          }
          auto art_ex = art::Exception{errors::ScheduleExecutionFailure,
                                       "Path: ProcessingStopped.",
                                       e}
                        << "Exception going through path " << name() << "\n";
          auto ex_ptr = make_exception_ptr(art_ex);
          waitingTasks_.load()->doneWaiting(ex_ptr);
          TDEBUG_END_TASK_SI_ERR(
            4, "workerDoneTask", sid, "terminate path because of EXCEPTION");
          return;
        }
        new_should_continue = false;
        mf::LogWarning(e.category())
          << "Failing path " << name() << ", due to exception, message:\n"
          << e.what() << "\n";
        // WARNING: We continue processing below!!!
      }
      catch (...) {
        mf::LogError("PassingThrough")
          << "Exception passing through path " << name() << "\n";
        ++timesExcept_;
        state_ = hlt::Exception;
        if (trptr_.load()) {
          // Not the end path.
          (*trptr_.load())[bitpos_] = HLTPathStatus(state_, idx);
        }
        waitingTasks_.load()->doneWaiting(current_exception());
        TDEBUG_END_TASK_SI_ERR(
          4, "workerDoneTask", sid, "terminate path because of EXCEPTION");
        return;
      }
      // WARNING: The only way we can get here is if the worker
      // threw and we are ignoring the exception but failing
      // the path because of actions::FailPath!!!
    }
    process_event_workerFinished(idx, max_idx, ep, new_should_continue);
    TDEBUG_END_TASK_SI(4, "workerDoneTask", sid);
  }

  // This function is the main body of the Run Worker task.
  // Note: Our parent task is the nullptr.
  void
  Path::process_event_idx(size_t const idx,
                          size_t const max_idx,
                          EventPrincipal& ep)
  {
    auto const sid = pc_.scheduleID();
    {
      ostringstream msg;
      msg << "idx: " << idx << " max_idx: " << max_idx;
      TDEBUG_FUNC_SI_MSG(4, "Path::process_event_idx", sid, msg.str());
    }
    auto workerDoneTask = make_waiting_task(
      tbb::task::allocate_root(), WorkerDoneFunctor{this, idx, max_idx, ep});
    auto& workerInPath = (*workers_.load())[idx];
    workerInPath.runWorker_event(workerDoneTask, ep);
    {
      ostringstream msg;
      msg << "idx: " << idx << " max_idx: " << max_idx;
      TDEBUG_FUNC_SI_MSG(4, "Path::process_event_idx", sid, msg.str());
    }
  }

  void
  Path::process_event_workerFinished(size_t const idx,
                                     size_t const max_idx,
                                     EventPrincipal& ep,
                                     bool const should_continue)
  {
    auto const sid = pc_.scheduleID();
    {
      ostringstream msg;
      msg << "idx: " << idx << " max_idx: " << max_idx
          << " should_continue: " << should_continue;
      TDEBUG_FUNC_SI_MSG(
        4, "Begin Path::process_event_workerFinished", sid, msg.str());
    }
    auto new_idx = idx;
    // Move on to the next worker.
    ++new_idx;
    if (should_continue && (new_idx < max_idx)) {
      // Spawn the next worker.
      process_event_idx_asynch(new_idx, max_idx, ep);
      // And end this one.
      {
        ostringstream msg;
        msg << "new_idx: " << new_idx << " max_idx: " << max_idx;
        TDEBUG_FUNC_SI_MSG(
          4, "Path::process_event_workerFinished", sid, msg.str());
      }
      return;
    }
    // All done, or filter rejected, or error.
    process_event_pathFinished(new_idx, ep, should_continue);
    // And end the path here.
    {
      ostringstream msg;
      msg << "idx: " << idx << " max_idx: " << max_idx;
      TDEBUG_FUNC_SI_MSG(
        4, "Path::process_event_workerFinished", sid, msg.str());
    }
  }

  // We come here as  as part of a runWorker task.
  // Our parent task is the nullptr.
  void
  Path::process_event_pathFinished(size_t const idx,
                                   EventPrincipal&,
                                   bool const should_continue)
  {
    auto const sid = pc_.scheduleID();
    {
      ostringstream msg;
      msg << "idx: " << idx << " should_continue: " << should_continue;
      TDEBUG_FUNC_SI_MSG(4, "Path::process_event_pathFinished", sid, msg.str());
    }
    try {
      if (should_continue) {
        ++timesPassed_;
        state_ = hlt::Pass;
      } else {
        ++timesFailed_;
        state_ = hlt::Fail;
      }
      if (trptr_.load()) {
        // Not the end path.
        (*trptr_.load())[bitpos_] = HLTPathStatus(state_, idx);
      }
      {
        HLTPathStatus const status(state_, idx);
        actReg_.load()->sPostProcessPath.invoke(pc_, status);
      }
    }
    catch (...) {
      waitingTasks_.load()->doneWaiting(current_exception());
      {
        ostringstream msg;
        msg << "idx: " << idx << " should_continue: " << should_continue
            << " EXCEPTION";
        TDEBUG_FUNC_SI_MSG(
          4, "Path::process_event_pathFinished", sid, msg.str());
      }
      return;
    }
    waitingTasks_.load()->doneWaiting(exception_ptr{});
    {
      ostringstream msg;
      msg << "idx: " << idx << " should_continue: " << should_continue;
      TDEBUG_FUNC_SI_MSG(4, "Path::process_event_pathFinished", sid, msg.str());
    }
  }

} // namespace art
