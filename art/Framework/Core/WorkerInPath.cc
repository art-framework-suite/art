#include "art/Framework/Core/WorkerInPath.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Principal/ExecutionCounts.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Utilities/TaskDebugMacros.h"
#include "art/Utilities/Transition.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskHolder.h"
#include "hep_concurrency/tsan.h"

#include <atomic>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

using namespace art::detail;
using namespace hep::concurrency;
using namespace std;

namespace art {

  WorkerInPath::~WorkerInPath() noexcept
  {
    worker_ = nullptr;
    delete waitingTasks_.load();
    waitingTasks_ = nullptr;
  }

  WorkerInPath::WorkerInPath(Worker* w,
                             FilterAction const fa,
                             ModuleContext const& mc)
    : moduleContext_{mc}
  {
    worker_ = w;
    filterAction_ = fa;
    returnCode_ = false;
    waitingTasks_ = new WaitingTaskList;
    counts_visited_ = 0;
    counts_passed_ = 0;
    counts_failed_ = 0;
    counts_thrown_ = 0;
  }

  WorkerInPath::WorkerInPath(WorkerInPath&& rhs)
    : moduleContext_{std::move(rhs.moduleContext_)}
  {
    worker_ = rhs.worker_.load();
    rhs.worker_ = nullptr;
    filterAction_ = rhs.filterAction_.load();
    returnCode_ = rhs.returnCode_.load();
    waitingTasks_ = rhs.waitingTasks_.load();
    rhs.waitingTasks_ = nullptr;
    counts_visited_ = rhs.counts_visited_.load();
    counts_passed_ = rhs.counts_passed_.load();
    counts_failed_ = rhs.counts_failed_.load();
    counts_thrown_ = rhs.counts_thrown_.load();
  }

  WorkerInPath&
  WorkerInPath::operator=(WorkerInPath&& rhs)
  {
    worker_ = rhs.worker_.load();
    rhs.worker_ = nullptr;
    filterAction_ = rhs.filterAction_.load();
    returnCode_ = rhs.returnCode_.load();
    waitingTasks_ = rhs.waitingTasks_.load();
    rhs.waitingTasks_ = nullptr;
    counts_visited_.store(rhs.counts_visited_.load());
    counts_passed_.store(rhs.counts_passed_.load());
    counts_failed_.store(rhs.counts_failed_.load());
    counts_thrown_.store(rhs.counts_thrown_.load());
    moduleContext_ = rhs.moduleContext_;
    return *this;
  }

  Worker*
  WorkerInPath::getWorker() const
  {
    return worker_.load();
  }

  detail::FilterAction
  WorkerInPath::filterAction() const
  {
    return filterAction_.load();
  }

  // Used only by Path
  bool
  WorkerInPath::returnCode() const
  {
    return returnCode_.load();
  }

  string const&
  WorkerInPath::label() const
  {
    return worker_.load()->label();
  }

  // Used only by Path
  void
  WorkerInPath::clearCounters()
  {
    counts_visited_ = 0;
    counts_passed_ = 0;
    counts_failed_ = 0;
    counts_thrown_ = 0;
  }

  // Used by writeSummary
  size_t
  WorkerInPath::timesVisited() const
  {
    return counts_visited_.load();
  }

  // Used by writeSummary
  size_t
  WorkerInPath::timesPassed() const
  {
    return counts_passed_.load();
  }

  // Used by writeSummary
  size_t
  WorkerInPath::timesFailed() const
  {
    return counts_failed_.load();
  }

  // Used by writeSummary
  size_t
  WorkerInPath::timesExcept() const
  {
    return counts_thrown_.load();
  }

  bool
  WorkerInPath::runWorker(Transition const trans, Principal& principal)
  {
    // Note: We ignore the return code because we do not process events here.
    worker_.load()->doWork(trans, principal, moduleContext_);
    return true;
  }

  void
  WorkerInPath::runWorker_event_for_endpath(EventPrincipal& ep)
  {
    auto const scheduleID = moduleContext_.scheduleID();
    TDEBUG_BEGIN_FUNC_SI(4, scheduleID);
    ++counts_visited_;
    try {
      worker_.load()->doWork_event(ep, moduleContext_);
    }
    catch (...) {
      ++counts_thrown_;
      TDEBUG_END_FUNC_SI(4, scheduleID) << "because of EXCEPTION";
      throw;
    }
    returnCode_ = worker_.load()->returnCode();
    TDEBUG_FUNC_SI(5, scheduleID) << "raw returnCode_: " << returnCode_.load();
    if (filterAction_.load() == FilterAction::Veto) {
      returnCode_ = !returnCode_.load();
    } else if (filterAction_.load() == FilterAction::Ignore) {
      returnCode_ = true;
    }
    TDEBUG_FUNC_SI(5, scheduleID)
      << "final returnCode_: " << returnCode_.load();
    if (returnCode_.load()) {
      ++counts_passed_;
    } else {
      ++counts_failed_;
    }
    TDEBUG_END_FUNC_SI(4, scheduleID);
  }

  class WorkerInPathDoneFunctor {
  public:
    WorkerInPathDoneFunctor(WorkerInPath* wip, ScheduleID const scheduleID)
      : wip_{wip}, sid_{scheduleID}
    {}
    void
    operator()(exception_ptr const* ex) const
    {
      wip_->workerInPathDoneTask(sid_, ex);
    }

  private:
    WorkerInPath* wip_;
    ScheduleID const sid_;
  };

  void
  WorkerInPath::workerInPathDoneTask(ScheduleID const scheduleID,
                                     exception_ptr const* ex)
  {
    TDEBUG_BEGIN_TASK_SI(4, scheduleID);
    if (ex != nullptr) {
      ++counts_thrown_;
      waitingTasks_.load()->doneWaiting(*ex);
      TDEBUG_END_TASK_SI(4, scheduleID) << "because of EXCEPTION";
      return;
    }
    returnCode_ = worker_.load()->returnCode();
    TDEBUG_TASK_SI(5, scheduleID) << "raw returnCode_: " << returnCode_.load();
    if (filterAction_.load() == FilterAction::Veto) {
      returnCode_ = !returnCode_.load();
    } else if (filterAction_.load() == FilterAction::Ignore) {
      returnCode_ = true;
    }
    TDEBUG_TASK_SI(5, scheduleID)
      << "final returnCode_: " << returnCode_.load();
    if (returnCode_.load()) {
      ++counts_passed_;
    } else {
      ++counts_failed_;
    }
    TDEBUG_END_TASK_SI(4, scheduleID) << "returnCode_: " << returnCode_.load();
    waitingTasks_.load()->doneWaiting(exception_ptr{});
  }

  void
  WorkerInPath::runWorker_event(WaitingTask* workerDoneTask, EventPrincipal& ep)
  {
    auto const scheduleID = moduleContext_.scheduleID();
    TDEBUG_BEGIN_FUNC_SI(4, scheduleID);
    // Reset the waiting task list so that it stops running tasks and switches
    // to holding them. Note: There will only ever be one entry in this list!
    TDEBUG_FUNC_SI(6, scheduleID)
      << hex << this << dec << " Resetting waitingTasks_";
    waitingTasks_.load()->reset();
    waitingTasks_.load()->add(workerDoneTask);
    ++counts_visited_;
    auto workerInPathDoneTask = make_waiting_task(
      tbb::task::allocate_root(), WorkerInPathDoneFunctor{this, scheduleID});
    try {
      worker_.load()->doWork_event(workerInPathDoneTask, ep, moduleContext_);
    }
    catch (...) {
      ++counts_thrown_;
      waitingTasks_.load()->doneWaiting(current_exception());
      TDEBUG_END_FUNC_SI(4, scheduleID) << "because of EXCEPTION";
      return;
    }
    TDEBUG_END_FUNC_SI(4, scheduleID);
  }

} // namespace art
