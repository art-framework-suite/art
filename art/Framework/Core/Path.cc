#include "art/Framework/Core/Path.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Utilities/CPCSentry.h"
#include "art/Utilities/CurrentProcessingContext.h"
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
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "tbb/task.h"

#include <algorithm>
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

  Path::Path(ActionTable& actions,
             ActivityRegistry& actReg,
             int const si,
             int const bitpos,
             bool const isEndPath,
             string const& path_name,
             vector<WorkerInPath>&& workers,
             HLTGlobalStatus* pathResults) noexcept
    : actionTable_{actions}
    , actReg_{actReg}
    , streamIndex_{si}
    , bitpos_{bitpos}
    , isEndPath_{isEndPath}
    , name_{path_name}
    , workers_{move(workers)}
    , trptr_{pathResults}
    , cpc_{si, &name_, bitpos, isEndPath}
    , waitingTasks_{}
  {
    TDEBUG(5) << "Path ctor: 0x" << hex << ((unsigned long)this) << dec << "\n";
  }

  int
  Path::streamIndex() const
  {
    return streamIndex_;
  }

  int
  Path::bitPosition() const
  {
    return bitpos_;
  }

  string const&
  Path::name() const
  {
    return name_;
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
  Path::process(Transition trans, Principal& principal)
  {
    if (trans == Transition::BeginRun) {
      actReg_.sPrePathBeginRun.invoke(name_);
    } else if (trans == Transition::EndRun) {
      actReg_.sPrePathEndRun.invoke(name_);
    } else if (trans == Transition::BeginSubRun) {
      actReg_.sPrePathBeginSubRun.invoke(name_);
    } else if (trans == Transition::EndSubRun) {
      actReg_.sPrePathEndSubRun.invoke(name_);
    }
    state_ = hlt::Ready;
    bool should_continue = true;
    std::size_t idx = 0;
    for (auto I = workers_.begin(), E = workers_.end();
         (I != E) && should_continue;
         ++I, ++idx) {
      try {
        cpc_.activate(idx, I->getWorker()->descPtr());
        should_continue = I->runWorker(trans, principal, &cpc_);
      }
      catch (cet::exception& e) {
        state_ = art::hlt::Exception;
        throw art::Exception{
          errors::ScheduleExecutionFailure, "Path: ProcessingStopped.", e}
          << "Exception going through path " << name_ << "\n";
      }
      catch (...) {
        mf::LogError("PassingThrough")
          << "Exception passing through path " << name_ << "\n";
        state_ = art::hlt::Exception;
        throw;
      }
    }
    if (should_continue) {
      state_ = art::hlt::Pass;
    } else {
      state_ = art::hlt::Fail;
    }
    HLTPathStatus const status(state_, idx);
    if (trans == Transition::BeginRun) {
      actReg_.sPostPathBeginRun.invoke(name_, status);
    } else if (trans == Transition::EndRun) {
      actReg_.sPostPathEndRun.invoke(name_, status);
    } else if (trans == Transition::BeginSubRun) {
      actReg_.sPostPathBeginSubRun.invoke(name_, status);
    } else if (trans == Transition::EndSubRun) {
      actReg_.sPostPathEndSubRun.invoke(name_, status);
    }
  }

  // We come here as part of the endPath task.  Our parent
  // is the eventLoop task.  We will run the workers on the
  // path in order serially without using tasks.
  void
  Path::process_event_for_endpath(EventPrincipal& ep, int si)
  {
    TDEBUG(4) << "-----> Begin Path::process_event_for_endpath (" << si
              << ") ...\n";
    detail::CPCSentry sentry{cpc_};
    actReg_.sPreProcessPath.invoke(name_);
    ++timesRun_;
    state_ = hlt::Ready;
    auto const max_idx = workers_.size();
    size_t idx = 0;
    bool should_continue = true;
    for (; should_continue && (idx < max_idx); ++idx) {
      auto& workerInPath = workers_[idx];
      cpc_.activate(idx, workerInPath.getWorker()->descPtr());
      try {
        workerInPath.runWorker_event_for_endpath(ep, si, &cpc_);
      }
      catch (cet::exception& e) {
        // Possible actions: IgnoreCompletely, Rethrow, SkipEvent, FailModule,
        // FailPath
        auto action = actionTable_.find(e.root_cause());
        assert(action != actions::FailModule);
        if (action != actions::FailPath) {
          // Possible actions: IgnoreCompletely, Rethrow, SkipEvent
          ++timesExcept_;
          state_ = art::hlt::Exception;
          if (trptr_) {
            // Not the end path (no trigger results for end path!).
            (*trptr_)[bitpos_] = HLTPathStatus(state_, idx);
          }
          TDEBUG(4) << "-----> End   Path::process_event_for_endpath (" << si
                    << ") ... terminate path because of EXCEPTION\n";
          throw Exception{
            errors::ScheduleExecutionFailure, "Path: ProcessingStopped.", e}
            << "Exception going through path " << name_ << "\n";
        }
        // Possible actions: FailPath
        should_continue = false;
        mf::LogWarning(e.category())
          << "Failing path " << name_ << ", due to exception, message:\n"
          << e.what() << "\n";
        // WARNING: We continue processing below!!!
        // WARNING: The only way we can get here is if the worker
        // threw and we are ignoring the exception but failing
        // the path because of actions::FailPath!!!
      }
      catch (...) {
        mf::LogError("PassingThrough")
          << "Exception passing through path " << name_ << "\n";
        ++timesExcept_;
        state_ = art::hlt::Exception;
        if (trptr_) {
          // Not the end path (no trigger results for end path!).
          (*trptr_)[bitpos_] = HLTPathStatus(state_, idx);
        }
        TDEBUG(4) << "-----> End   Path::process_event_for_endpath (" << si
                  << ") ... terminate end path because of EXCEPTION\n";
        return;
      }
      // Note: This will only be set false by a filter which has rejected
      // (impossible on the end path!).
      should_continue = workerInPath.returnCode(si);
      TDEBUG(5) << "-----> process_event_for_endpath: si: " << si
                << " idx: " << idx << " should_continue: " << should_continue
                << "\n";
    }
    // All done, or filter rejected, or error.
    try {
      if (should_continue) {
        ++timesPassed_;
        state_ = art::hlt::Pass;
      } else {
        ++timesFailed_;
        state_ = art::hlt::Fail;
      }
      if (trptr_) {
        // Not the end path.
        (*trptr_)[bitpos_] = HLTPathStatus(state_, idx);
      }
      HLTPathStatus const status(state_, idx);
      actReg_.sPostProcessPath.invoke(name_, status);
    }
    catch (...) {
      TDEBUG(4)
        << "-----> End   Path::process_event_for_endpath (" << si
        << ") ... terminate end path final processing because of EXCEPTION\n";
      throw;
    }
    TDEBUG(4) << "-----> End   Path::process_event_for_endpath (" << si
              << ") ...\n";
  }

  // We come here as part of the readAndProcessEvent task, or
  // as part of the endPath task.  Our parent is the nullptr.
  // The parent of the pathsDoneTask is the eventLoop task.
  void
  Path::process_event(WaitingTask* pathsDoneTask, EventPrincipal& ep, int si)
  {
    TDEBUG(4) << "-----> Begin Path::process_event (" << si << ") ...\n";
    {
      ostringstream buf;
      buf << "-----> Path::process_event: 0x" << hex << ((unsigned long)this)
          << dec << " Resetting waitingTasks_ (" << si << ") ...\n";
      TDEBUG(6) << buf.str();
    }
    // Make sure the list is not auto-spawning tasks.
    waitingTasks_.reset();
    // Note: This task list will never have more than one entry.
    waitingTasks_.add(pathsDoneTask);
    detail::CPCSentry sentry{cpc_};
    actReg_.sPreProcessPath.invoke(name_);
    ++timesRun_;
    state_ = hlt::Ready;
    size_t idx = 0;
    auto max_idx = workers_.size();
    bool should_continue = true;
    // Start the task spawn chain going with the first worker on the
    // path.  Each worker will spawn the next worker in order, until
    // all the workers have run.
    process_event_idx_asynch(idx, max_idx, ep, si, should_continue, &cpc_);
    TDEBUG(4) << "-----> End   Path::process_event (" << si << ") ...\n";
  }

  // This function is a spawn chain system to run workers one at a time,
  // in the order specified on the path, and then decrement the ref count
  // on the endPathsTask when finished (which causes it to run if we are
  // the last path to finish running its workers).
  void
  Path::process_event_idx_asynch(size_t idx,
                                 size_t const max_idx,
                                 EventPrincipal& ep,
                                 int si,
                                 bool should_continue,
                                 CurrentProcessingContext* cpc)
  {
    TDEBUG(4) << "-----> Begin Path::process_event_idx_asynch: si: " << si
              << " idx: " << idx << " max_idx: " << max_idx << " ...\n";
    auto runWorkerTaskFunctor =
      [this, idx, max_idx, &ep, si, should_continue, cpc](
        std::exception_ptr const* /*unused*/) {
        // Note: When we start here our parent task is the nullptr.
        TDEBUG(4) << "=====> Begin runWorkerTask (" << si << ") ...\n";
        auto new_idx = idx;
        auto new_should_continue = should_continue;
        auto new_cpc = cpc;
        try {
          process_event_idx(
            new_idx, max_idx, ep, si, new_should_continue, new_cpc);
        }
        catch (...) {
          waitingTasks_.doneWaiting(current_exception());
          // End this task, terminating the path here.
          TDEBUG(4) << "=====> End   runWorkerTask (" << si
                    << ") ... path terminate because of exception\n";
          return;
        }
        TDEBUG(4) << "=====> End   runWorkerTask (" << si << ") ...\n";
      };
    auto runWorkerTask =
      make_waiting_task(tbb::task::allocate_root(), runWorkerTaskFunctor);
    tbb::task::spawn(*runWorkerTask);
    // And end this task, which does not terminate event
    // processing because our parent task is the nullptr.
    TDEBUG(4) << "-----> End   Path::process_event_idx_asynch: si: " << si
              << " idx: " << idx << " max_idx: " << max_idx << " ...\n";
  }

  // This function is the main body of the Run Worker task.
  // Note: Our parent task is the nullptr.
  void
  Path::process_event_idx(size_t const idx,
                          size_t const max_idx,
                          EventPrincipal& ep,
                          int si,
                          bool const should_continue,
                          CurrentProcessingContext* cpc)
  {
    TDEBUG(4) << "-----> Begin Path::process_event_idx: si: " << si
              << " idx: " << idx << " max_idx: " << max_idx << " ...\n";
    auto workerDoneFunctor =
      [this, idx, max_idx, &ep, si, should_continue, cpc](
        exception_ptr const* ex) mutable {
        TDEBUG(4) << "=====> Begin workerDoneTask (" << si << ") ...\n";
        auto& workerInPath = workers_[idx];
        // Note: This will only be set false by a filter which has rejected.
        bool new_should_continue = workerInPath.returnCode(si);
        TDEBUG(4) << "=====> workerDoneTask: si: " << si
                  << " new_should_continue: " << new_should_continue << "\n";
        if (ex != nullptr) {
          try {
            rethrow_exception(*ex);
          }
          catch (cet::exception& e) {
            auto action = actionTable_.find(e.root_cause());
            assert(action != actions::FailModule);
            if (action != actions::FailPath) {
              ++timesExcept_;
              state_ = art::hlt::Exception;
              if (trptr_) {
                // Not the end path.
                (*trptr_)[bitpos_] = HLTPathStatus(state_, idx);
              }
              auto art_ex = art::Exception{errors::ScheduleExecutionFailure,
                                           "Path: ProcessingStopped.",
                                           e}
                            << "Exception going through path " << name_ << "\n";
              auto ex_ptr = make_exception_ptr(art_ex);
              waitingTasks_.doneWaiting(ex_ptr);
              TDEBUG(4) << "=====> End   workerDoneTask (" << si
                        << ") ... terminate path because of exception\n";
              return;
            }
            new_should_continue = false;
            mf::LogWarning(e.category())
              << "Failing path " << name_ << ", due to exception, message:\n"
              << e.what() << "\n";
            // WARNING: We continue processing below!!!
          }
          catch (...) {
            mf::LogError("PassingThrough")
              << "Exception passing through path " << name_ << "\n";
            ++timesExcept_;
            state_ = art::hlt::Exception;
            if (trptr_) {
              // Not the end path.
              (*trptr_)[bitpos_] = HLTPathStatus(state_, idx);
            }
            waitingTasks_.doneWaiting(current_exception());
            TDEBUG(4) << "=====> End   workerDoneTask (" << si
                      << ") ... terminate path because of exception\n";
            return;
          }
          // WARNING: The only way we can get here is if the worker
          // threw and we are ignoring the exception but failing
          // the path because of actions::FailPath!!!
        }
        process_event_workerFinished(
          idx, max_idx, ep, si, new_should_continue, cpc);
        TDEBUG(4) << "=====> End   workerDoneTask ...\n";
        return;
      };
    auto workerDoneTask =
      make_waiting_task(tbb::task::allocate_root(), workerDoneFunctor);
    auto& workerInPath = workers_[idx];
    cpc->activate(idx, workerInPath.getWorker()->descPtr());
    workerInPath.runWorker_event(workerDoneTask, ep, si, cpc);
    TDEBUG(4) << "-----> End   Path::process_event_idx: si: " << si
              << " idx: " << idx << " max_idx: " << max_idx << " ...\n";
  }

  void
  Path::process_event_workerFinished(size_t const idx,
                                     size_t const max_idx,
                                     EventPrincipal& ep,
                                     int si,
                                     bool const should_continue,
                                     CurrentProcessingContext* cpc)
  {
    TDEBUG(4) << "-----> Begin Path::process_event_workerFinished: si: " << si
              << " idx: " << idx << " max_idx: " << max_idx
              << " should_continue: " << should_continue << " ...\n";
    auto new_idx = idx;
    // Move on to the next worker.
    ++new_idx;
    if (should_continue && (new_idx < max_idx)) {
      // Spawn the next worker.
      process_event_idx_asynch(new_idx, max_idx, ep, si, should_continue, cpc);
      // And end this one.
      TDEBUG(4) << "-----> End   Path::process_event_workerFinished: si: " << si
                << " new_idx: " << new_idx << " max_idx: " << max_idx << "\n";
      return;
    }
    // All done, or filter rejected, or error.
    process_event_pathFinished(new_idx, ep, si, should_continue, cpc);
    // And end the path here.
    TDEBUG(4) << "-----> End   Path::process_event_workerFinished: si: " << si
              << " new_idx: " << new_idx << " max_idx: " << max_idx << "\n";
  }

  // We come here as  as part of a runWorker task.
  // Our parent task is the nullptr.
  void
  Path::process_event_pathFinished(size_t const idx,
                                   EventPrincipal& /*ep*/,
                                   int si,
                                   bool const should_continue,
                                   CurrentProcessingContext* /*cpc*/)
  {
    TDEBUG(4) << "-----> Begin Path::process_event_pathFinished: si: " << si
              << " idx: " << idx << " should_continue: " << should_continue
              << " ...\n";
    try {
      if (should_continue) {
        ++timesPassed_;
        state_ = art::hlt::Pass;
      } else {
        ++timesFailed_;
        state_ = art::hlt::Fail;
      }
      if (trptr_) {
        // Not the end path.
        (*trptr_)[bitpos_] = HLTPathStatus(state_, idx);
      }
      HLTPathStatus const status(state_, idx);
      actReg_.sPostProcessPath.invoke(name_, status);
    }
    catch (...) {
      waitingTasks_.doneWaiting(current_exception());
      TDEBUG(4) << "-----> End   Path::process_event_pathFinished: si: " << si
                << " idx: " << idx << " should_continue: " << should_continue
                << " ...\n";
      return;
    }
    waitingTasks_.doneWaiting(exception_ptr{});
    TDEBUG(4) << "-----> End   Path::process_event_pathFinished: si: " << si
              << " idx: " << idx << " should_continue: " << should_continue
              << " ...\n";
  }

} // namespace art
