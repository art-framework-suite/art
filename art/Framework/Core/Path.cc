#include "art/Framework/Core/Path.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Core/detail/skip_non_replicated.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/TaskDebugMacros.h"
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "canvas/Persistency/Common/HLTPathStatus.h"
#include "canvas/Persistency/Common/HLTenums.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/tsan.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "tbb/task.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <exception>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace cet;
using namespace fhicl;
using namespace hep::concurrency;
using namespace std;

namespace art {

  Path::Path(ActionTable const& actions,
             ActivityRegistry const& actReg,
             PathContext const& pc,
             vector<WorkerInPath>&& workers,
             HLTGlobalStatus* pathResults) noexcept
    : actionTable_{&actions}
    , actReg_{&actReg}
    , pc_{pc}
    , bitpos_{pc.bitPosition()}
    , workers_{move(workers)}
    , trptr_{pathResults}
  {
    TDEBUG_FUNC_SI(4, pc_.scheduleID()) << hex << this << dec;
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
    return workers_;
  }

  void
  Path::clearCounters()
  {
    timesRun_ = 0;
    timesPassed_ = 0;
    timesFailed_ = 0;
    timesExcept_ = 0;
    for (auto& w : workers_) {
      w.clearCounters();
    }
  }

  void
  Path::process(Transition const trans, Principal& principal)
  {
    // Invoke pre-path signals only for the first schedule.
    if (pc_.scheduleID() == ScheduleID::first()) {
      switch (trans) {
        case Transition::BeginRun:
          actReg_.load()->sPrePathBeginRun.invoke(name());
          break;
        case Transition::EndRun:
          actReg_.load()->sPrePathEndRun.invoke(name());
          break;
        case Transition::BeginSubRun:
          actReg_.load()->sPrePathBeginSubRun.invoke(name());
          break;
        case Transition::EndSubRun:
          actReg_.load()->sPrePathEndSubRun.invoke(name());
          break;
        default: {} // No other pre-path signals supported.
      }
    }
    state_ = hlt::Ready;
    std::size_t idx = 0;
    bool all_passed{false};
    for (WorkerInPath& wip : workers_) {
      // We do not want to call (e.g.) beginRun once per schedule for
      // non-replicated modules.
      if (detail::skip_non_replicated(*wip.getWorker())) {
        continue;
      }
      try {
        all_passed = wip.runWorker(trans, principal);
        if (!all_passed)
          break;
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
      ++idx;
    }
    if (all_passed) {
      state_ = hlt::Pass;
    } else {
      state_ = hlt::Fail;
    }
    // Invoke post-path signals only for the last schedule.
    if (pc_.scheduleID().id() == art::Globals::instance()->nschedules() - 1) {
      HLTPathStatus const status(state_, idx);
      switch (trans) {
        case Transition::BeginRun:
          actReg_.load()->sPostPathBeginRun.invoke(name(), status);
          break;
        case Transition::EndRun:
          actReg_.load()->sPostPathEndRun.invoke(name(), status);
          break;
        case Transition::BeginSubRun:
          actReg_.load()->sPostPathBeginSubRun.invoke(name(), status);
          break;
        case Transition::EndSubRun:
          actReg_.load()->sPostPathEndSubRun.invoke(name(), status);
          break;
        default: {} // No other post-path signals supported.
      }
    }
  }

  // We come here as part of the endPath task.  Our parent is the
  // eventLoop task.  We will run the workers on the path in order
  // serially without using tasks.
  void
  Path::process_event_for_endpath(EventPrincipal& ep)
  {
    auto const sid = pc_.scheduleID();
    TDEBUG_BEGIN_FUNC_SI(4, sid);
    actReg_.load()->sPreProcessPath.invoke(pc_);
    ++timesRun_;
    state_ = hlt::Ready;
    auto const max_idx = workers_.size();
    size_t idx = 0;
    bool should_continue = true;
    for (; should_continue && (idx < max_idx); ++idx) {
      auto& workerInPath = workers_[idx];
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
          TDEBUG_END_FUNC_SI(4, sid) << "terminate path because of EXCEPTION";
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
        TDEBUG_END_FUNC_SI(4, sid) << "terminate end path because of EXCEPTION";
        return;
      }
      // Note: This will only be set false by a filter which has rejected
      // (impossible on the end path!).
      should_continue = workerInPath.returnCode();
      TDEBUG_FUNC_SI(5, sid)
        << "idx: " << idx << " should_continue: " << should_continue;
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
      TDEBUG_END_FUNC_SI(4, sid)
        << "terminate end path final processing because of EXCEPTION";
      throw;
    }
    TDEBUG_END_FUNC_SI(4, sid);
  }

  // We come here as part of the readAndProcessEvent task, or as part
  // of the endPath task.  Our parent is the nullptr.  The parent of
  // the pathsDoneTask is the eventLoop task.
  void
  Path::process_event(tbb::task* pathsDoneTask, EventPrincipal& ep)
  {
    auto const sid = pc_.scheduleID();
    // Note: We are part of the readAndProcessEventTask (stream head task),
    // or the endPath task, and our parent task is the nullptr.
    // The parent of the pathsDoneTask is the eventLoop Task.
    TDEBUG_BEGIN_FUNC_SI(4, sid);
    TDEBUG_FUNC_SI(6, sid) << hex << this << dec << " Resetting waitingTasks_";

    // Make sure the list is not auto-spawning tasks.
    waitingTasks_.reset();
    // Note: This task list will never have more than one entry.
    waitingTasks_.add(pathsDoneTask);
    {
      actReg_.load()->sPreProcessPath.invoke(pc_);
    }
    ++timesRun_;
    state_ = hlt::Ready;
    size_t idx = 0;
    auto max_idx = workers_.size();
    // Start the task spawn chain going with the first worker on the
    // path.  Each worker will spawn the next worker in order, until
    // all the workers have run.
    process_event_idx_asynch(idx, max_idx, ep);
    TDEBUG_END_FUNC_SI(4, sid);
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
    TDEBUG_BEGIN_TASK_SI(4, sid);
    auto new_idx = idx;
    try {
      process_event_idx(new_idx, max_idx, ep);
    }
    catch (...) {
      waitingTasks_.doneWaiting(current_exception());
      // End this task, terminating the path here.
      TDEBUG_END_TASK_SI(4, sid) << "path terminate because of EXCEPTION";
      return;
    }
    TDEBUG_END_TASK_SI(4, sid);
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
    TDEBUG_BEGIN_FUNC_SI(4, sid) << "idx: " << idx << " max_idx: " << max_idx;
    auto runWorkerTask = make_waiting_task(
      tbb::task::allocate_root(), RunWorkerFunctor{this, idx, max_idx, ep});
    tbb::task::spawn(*runWorkerTask);
    // And end this task, which does not terminate event processing
    // because our parent task is the nullptr.
    TDEBUG_FUNC_SI(4, sid) << "idx: " << idx << " max_idx: " << max_idx;
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
    TDEBUG_BEGIN_TASK_SI(4, sid);
    auto& workerInPath = workers_[idx];
    // Note: This will only be set false by a filter which has rejected.
    bool new_should_continue = workerInPath.returnCode();
    TDEBUG_TASK_SI(4, sid) << "new_should_continue: " << new_should_continue;
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
          waitingTasks_.doneWaiting(ex_ptr);
          TDEBUG_END_TASK_SI(4, sid) << "terminate path because of EXCEPTION";
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
        waitingTasks_.doneWaiting(current_exception());
        TDEBUG_END_TASK_SI(4, sid) << "terminate path because of EXCEPTION";
        return;
      }
      // WARNING: The only way we can get here is if the worker
      // threw and we are ignoring the exception but failing
      // the path because of actions::FailPath!!!
    }
    process_event_workerFinished(idx, max_idx, ep, new_should_continue);
    TDEBUG_END_TASK_SI(4, sid);
  }

  // This function is the main body of the Run Worker task.
  // Note: Our parent task is the nullptr.
  void
  Path::process_event_idx(size_t const idx,
                          size_t const max_idx,
                          EventPrincipal& ep)
  {
    auto const sid = pc_.scheduleID();
    TDEBUG_FUNC_SI(4, sid) << "idx: " << idx << " max_idx: " << max_idx;
    auto workerDoneTask = make_waiting_task(
      tbb::task::allocate_root(), WorkerDoneFunctor{this, idx, max_idx, ep});
    auto& workerInPath = workers_[idx];
    workerInPath.runWorker_event(workerDoneTask, ep);
    TDEBUG_FUNC_SI(4, sid) << "idx: " << idx << " max_idx: " << max_idx;
  }

  void
  Path::process_event_workerFinished(size_t const idx,
                                     size_t const max_idx,
                                     EventPrincipal& ep,
                                     bool const should_continue)
  {
    auto const sid = pc_.scheduleID();
    TDEBUG_BEGIN_FUNC_SI(4, sid) << "idx: " << idx << " max_idx: " << max_idx
                                 << " should_continue: " << should_continue;
    auto new_idx = idx;
    // Move on to the next worker.
    ++new_idx;
    if (should_continue && (new_idx < max_idx)) {
      // Spawn the next worker.
      process_event_idx_asynch(new_idx, max_idx, ep);
      // And end this one.
      TDEBUG_FUNC_SI(4, sid)
        << "new_idx: " << new_idx << " max_idx: " << max_idx;
      return;
    }
    // All done, or filter rejected, or error.
    process_event_pathFinished(new_idx, ep, should_continue);
    // And end the path here.
    TDEBUG_FUNC_SI(4, sid) << "idx: " << idx << " max_idx: " << max_idx;
  }

  // We come here as as part of a runWorker task.  Our parent task is
  // the nullptr.
  void
  Path::process_event_pathFinished(size_t const idx,
                                   EventPrincipal&,
                                   bool const should_continue)
  {
    auto const sid = pc_.scheduleID();
    TDEBUG_FUNC_SI(4, sid) << "idx: " << idx
                           << " should_continue: " << should_continue;
    bool exception = false;
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
      exception = true;
    }
    waitingTasks_.doneWaiting(exception_ptr{});
    TDEBUG_FUNC_SI(4, sid) << "idx: " << idx
                           << " should_continue: " << should_continue
                           << (exception ? " EXCEPTION" : "");
  }

} // namespace art
