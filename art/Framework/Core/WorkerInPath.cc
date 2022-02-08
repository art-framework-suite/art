#include "art/Framework/Core/WorkerInPath.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/GlobalTaskGroup.h"
#include "art/Utilities/TaskDebugMacros.h"
#include "art/Utilities/Transition.h"
#include "hep_concurrency/WaitingTask.h"

using namespace art::detail;
using namespace hep::concurrency;
using namespace std;

namespace art {

  WorkerInPath::WorkerInPath(cet::exempt_ptr<Worker> w,
                             FilterAction const fa,
                             ModuleContext const& mc,
                             GlobalTaskGroup& taskGroup)
    : worker_{w}, filterAction_{fa}, moduleContext_{mc}, taskGroup_{&taskGroup}
  {}

  Worker*
  WorkerInPath::getWorker() const
  {
    return worker_.get();
  }

  detail::FilterAction
  WorkerInPath::filterAction() const
  {
    return filterAction_;
  }

  // Used only by Path
  bool
  WorkerInPath::returnCode() const
  {
    return returnCode_;
  }

  // Used by writeSummary
  size_t
  WorkerInPath::timesVisited() const
  {
    return counts_visited_;
  }

  // Used by writeSummary
  size_t
  WorkerInPath::timesPassed() const
  {
    return counts_passed_;
  }

  // Used by writeSummary
  size_t
  WorkerInPath::timesFailed() const
  {
    return counts_failed_;
  }

  // Used by writeSummary
  size_t
  WorkerInPath::timesExcept() const
  {
    return counts_thrown_;
  }

  bool
  WorkerInPath::run(Transition const trans, Principal& principal)
  {
    // Note: We ignore the return code because we do not process events here.
    worker_->doWork(trans, principal, moduleContext_);
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

      wip_->returnCode_ = wip_->worker_->returnCode();
      TDEBUG_TASK_SI(5, sid_) << "raw returnCode_: " << wip_->returnCode_;
      if (wip_->filterAction_ == FilterAction::Veto) {
        wip_->returnCode_ = !wip_->returnCode_;
      } else if (wip_->filterAction_ == FilterAction::Ignore) {
        wip_->returnCode_ = true;
      }
      TDEBUG_TASK_SI(5, sid_) << "final returnCode_: " << wip_->returnCode_;
      if (wip_->returnCode_) {
        ++wip_->counts_passed_;
      } else {
        ++wip_->counts_failed_;
      }
      TDEBUG_END_TASK_SI(4, sid_) << "returnCode_: " << wip_->returnCode_;
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
      worker_->doWork_event(workerInPathDoneTask, ep, moduleContext_);
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
