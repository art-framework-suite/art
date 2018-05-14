#include "art/Framework/Core/WorkerInPath.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Principal/ExecutionCounts.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Utilities/Transition.h"
#include "canvas/Utilities/DebugMacros.h"
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

using namespace hep::concurrency;
using namespace std;

namespace art {

  WorkerInPath::~WorkerInPath() noexcept
  {
    worker_ = nullptr;
    delete waitingTasks_.load();
    waitingTasks_ = nullptr;
  }

  WorkerInPath::WorkerInPath(Worker* w) noexcept
    : WorkerInPath{w, Normal}
  {}

  WorkerInPath::WorkerInPath(Worker* w, FilterAction const fa) noexcept
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

  WorkerInPath::WorkerInPath(WorkerInPath&& rhs) noexcept
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
  WorkerInPath::operator=(WorkerInPath&& rhs) noexcept
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
    return *this;
  }

  Worker*
  WorkerInPath::getWorker() const
  {
    return worker_.load();
  }

  WorkerInPath::FilterAction
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
  WorkerInPath::runWorker(Transition const trans,
                          Principal& principal,
                          PathContext const& pc)
  {
    // Note: We ignore the return code because we do not process events here.
    worker_.load()->doWork(trans, principal, pc);
    return true;
  }

  void
  WorkerInPath::runWorker_event_for_endpath(EventPrincipal& ep,
                                            PathContext const& pc)
  {
    auto const scheduleID = pc.scheduleID();
    TDEBUG_BEGIN_FUNC_SI(
      4, "WorkerInPath::runWorker_event_for_endpath", scheduleID);
    ++counts_visited_;
    try {
      worker_.load()->doWork_event(ep, pc);
    }
    catch (...) {
      ++counts_thrown_;
      TDEBUG_END_FUNC_SI_ERR(4,
                             "WorkerInPath::runWorker_event_for_endpath",
                             scheduleID,
                             "because of EXCEPTION");
      throw;
    }
    returnCode_ = worker_.load()->returnCode(scheduleID);
    {
      ostringstream msg;
      msg << "raw returnCode_: " << returnCode_.load();
      TDEBUG_FUNC_SI_MSG(
        5, "WorkerInPath::runWorker_event_for_endpath", scheduleID, msg.str());
    }
    if (filterAction_.load() == Veto) {
      returnCode_ = !returnCode_.load();
    } else if (filterAction_.load() == Ignore) {
      returnCode_ = true;
    }
    {
      ostringstream msg;
      msg << "final returnCode_: " << returnCode_.load();
      TDEBUG_FUNC_SI_MSG(
        5, "WorkerInPath::runWorker_event_for_endpath", scheduleID, msg.str());
    }
    if (returnCode_.load()) {
      ++counts_passed_;
    } else {
      ++counts_failed_;
    }
    TDEBUG_END_FUNC_SI(
      4, "WorkerInPath::runWorker_event_for_endpath", scheduleID);
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
    TDEBUG_BEGIN_TASK_SI(4, "workerInPathDoneTask", scheduleID);
    if (ex != nullptr) {
      ++counts_thrown_;
      waitingTasks_.load()->doneWaiting(*ex);
      TDEBUG_END_TASK_SI_ERR(
        4, "workerInPathDoneTask", scheduleID, "because of EXCEPTION");
      return;
    }
    returnCode_ = worker_.load()->returnCode(scheduleID);
    {
      ostringstream msg;
      msg << "raw returnCode_: " << returnCode_.load();
      TDEBUG_TASK_SI_MSG(5, "workerInPathDoneTask", scheduleID, msg.str());
    }
    if (filterAction_.load() == Veto) {
      returnCode_ = !returnCode_.load();
    } else if (filterAction_.load() == Ignore) {
      returnCode_ = true;
    }
    {
      ostringstream msg;
      msg << "final returnCode_: " << returnCode_.load();
      TDEBUG_TASK_SI_MSG(5, "workerInPathDoneTask", scheduleID, msg.str());
    }
    if (returnCode_.load()) {
      ++counts_passed_;
    } else {
      ++counts_failed_;
    }
    {
      ostringstream msg;
      msg << "returnCode_: " << returnCode_.load();
      TDEBUG_END_TASK_SI_ERR(4, "workerInPathDoneTask", scheduleID, msg.str());
    }
    waitingTasks_.load()->doneWaiting(exception_ptr{});
  }

  void
  WorkerInPath::runWorker_event(WaitingTask* workerDoneTask,
                                EventPrincipal& ep,
                                PathContext const& pc)
  {
    auto const scheduleID = pc.scheduleID();
    TDEBUG_BEGIN_FUNC_SI(4, "WorkerInPath::runWorker_event", scheduleID);
    // Reset the waiting task list so that it stops running tasks and switches
    // to holding them. Note: There will only ever be one entry in this list!
    {
      ostringstream msg;
      msg << "0x" << hex << ((unsigned long)this) << dec
          << " Resetting waitingTasks_";
      TDEBUG_FUNC_SI_MSG(
        6, "WorkerInPath::runWorker_event", scheduleID, msg.str());
    }
    waitingTasks_.load()->reset();
    waitingTasks_.load()->add(workerDoneTask);
    ++counts_visited_;
    auto workerInPathDoneTask = make_waiting_task(
      tbb::task::allocate_root(), WorkerInPathDoneFunctor{this, scheduleID});
    try {
      worker_.load()->doWork_event(workerInPathDoneTask, ep, pc);
    }
    catch (...) {
      ++counts_thrown_;
      waitingTasks_.load()->doneWaiting(current_exception());
      TDEBUG_END_FUNC_SI_ERR(
        4, "WorkerInPath::runWorker_event", scheduleID, "because of EXCEPTION");
      return;
    }
    TDEBUG_END_FUNC_SI(4, "WorkerInPath::runWorker_event", scheduleID);
  }

} // namespace art
