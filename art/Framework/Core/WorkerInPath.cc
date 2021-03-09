#include "art/Framework/Core/WorkerInPath.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Principal/ExecutionCounts.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Utilities/GlobalTaskGroup.h"
#include "art/Utilities/TaskDebugMacros.h"
#include "art/Utilities/Transition.h"
#include "hep_concurrency/WaitingTask.h"

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

  WorkerInPath::WorkerInPath(Worker* w,
                             FilterAction const fa,
                             ModuleContext const& mc,
                             GlobalTaskGroup& taskGroup)
    : worker_{w}, filterAction_{fa}, moduleContext_{mc}, taskGroup_{&taskGroup}
  {}

  WorkerInPath::WorkerInPath(WorkerInPath&& rhs)
    : worker_{rhs.worker_.load()}
    , filterAction_{rhs.filterAction_.load()}
    , moduleContext_{std::move(rhs.moduleContext_)}
    , taskGroup_{rhs.taskGroup_}
  {
    returnCode_ = rhs.returnCode_.load();
    counts_visited_ = rhs.counts_visited_.load();
    counts_passed_ = rhs.counts_passed_.load();
    counts_failed_ = rhs.counts_failed_.load();
    counts_thrown_ = rhs.counts_thrown_.load();
    rhs.worker_ = nullptr;
  }

  WorkerInPath&
  WorkerInPath::operator=(WorkerInPath&& rhs)
  {
    worker_ = rhs.worker_.load();
    rhs.worker_ = nullptr;
    filterAction_ = rhs.filterAction_.load();
    returnCode_ = rhs.returnCode_.load();
    counts_visited_.store(rhs.counts_visited_.load());
    counts_passed_.store(rhs.counts_passed_.load());
    counts_failed_.store(rhs.counts_failed_.load());
    counts_thrown_.store(rhs.counts_thrown_.load());
    moduleContext_ = rhs.moduleContext_;
    taskGroup_ = std::exchange(rhs.taskGroup_, nullptr);
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
  WorkerInPath::run(Transition const trans, Principal& principal)
  {
    // Note: We ignore the return code because we do not process events here.
    worker_.load()->doWork(trans, principal, moduleContext_);
    return true;
  }

  class WorkerInPath::WorkerInPathDoneTask {
  public:
    WorkerInPathDoneTask(WorkerInPath* wip,
                         ScheduleID const scheduleID,
                         WaitingTaskPtr workerDoneTask,
                         GlobalTaskGroup* taskGroup)
      : wip_{wip}
      , sid_{scheduleID}
      , workerDoneTask_{std::move(workerDoneTask)}
      , taskGroup_{taskGroup}
    {}

    void
    operator()(exception_ptr const ex) const
    {
      TDEBUG_BEGIN_TASK_SI(4, sid_);
      if (ex) {
        ++wip_->counts_thrown_;
        taskGroup_->may_run(workerDoneTask_, ex);
        TDEBUG_END_TASK_SI(4, sid_) << "because of EXCEPTION";
        return;
      }

      wip_->returnCode_ = wip_->worker_.load()->returnCode();
      TDEBUG_TASK_SI(5, sid_)
        << "raw returnCode_: " << wip_->returnCode_.load();
      if (wip_->filterAction_.load() == FilterAction::Veto) {
        wip_->returnCode_ = !wip_->returnCode_.load();
      } else if (wip_->filterAction_.load() == FilterAction::Ignore) {
        wip_->returnCode_ = true;
      }
      TDEBUG_TASK_SI(5, sid_)
        << "final returnCode_: " << wip_->returnCode_.load();
      if (wip_->returnCode_.load()) {
        ++wip_->counts_passed_;
      } else {
        ++wip_->counts_failed_;
      }
      TDEBUG_END_TASK_SI(4, sid_)
        << "returnCode_: " << wip_->returnCode_.load();
      taskGroup_->may_run(workerDoneTask_);
    }

  private:
    WorkerInPath* wip_;
    ScheduleID const sid_;
    WaitingTaskPtr workerDoneTask_;
    GlobalTaskGroup* taskGroup_;
  };

  void
  WorkerInPath::run(WaitingTaskPtr workerDoneTask, EventPrincipal& ep)
  {
    auto const scheduleID = moduleContext_.scheduleID();
    TDEBUG_BEGIN_FUNC_SI(4, scheduleID);
    ++counts_visited_;
    try {
      auto workerInPathDoneTask = make_waiting_task<WorkerInPathDoneTask>(
        this, scheduleID, workerDoneTask, taskGroup_);
      worker_.load()->doWork_event(workerInPathDoneTask, ep, moduleContext_);
    }
    catch (...) {
      ++counts_thrown_;
      taskGroup_->may_run(workerDoneTask, current_exception());
      TDEBUG_END_FUNC_SI(4, scheduleID) << "because of EXCEPTION";
      return;
    }
    TDEBUG_END_FUNC_SI(4, scheduleID);
  }

} // namespace art
