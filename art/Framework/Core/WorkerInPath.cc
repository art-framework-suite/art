#include "art/Framework/Core/WorkerInPath.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Principal/ExecutionCounts.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Utilities/Transition.h"
#include "canvas/Utilities/DebugMacros.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskHolder.h"

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

WorkerInPath::
~WorkerInPath() noexcept
{
  worker_ = nullptr;
}

WorkerInPath::
WorkerInPath(Worker* w) noexcept
  : worker_{w}
  , filterAction_{Normal}
  , returnCode_{false}
  , waitingTasks_{}
  , counts_visited_{}
  , counts_passed_{}
  , counts_failed_{}
  , counts_thrown_{}
{
}

WorkerInPath::
WorkerInPath(Worker* w, FilterAction const fa) noexcept
  : worker_{w}
  , filterAction_{fa}
  , returnCode_{false}
  , waitingTasks_{}
  , counts_visited_{}
  , counts_passed_{}
  , counts_failed_{}
  , counts_thrown_{}
{
}

WorkerInPath::
WorkerInPath(WorkerInPath&& rhs) noexcept
  : worker_{move(rhs.worker_)}
  , filterAction_{move(rhs.filterAction_)}
  , returnCode_{move(rhs.returnCode_)}
  // Note: A WaitingTaskList cannot be moved,
  // but it only happens at system startup,
  // so oh well.
  , waitingTasks_{}
  , counts_visited_{rhs.counts_visited_.load()}
  , counts_passed_{rhs.counts_passed_.load()}
  , counts_failed_{rhs.counts_failed_.load()}
  , counts_thrown_{rhs.counts_thrown_.load()}
{
}

WorkerInPath&
WorkerInPath::
operator=(WorkerInPath&& rhs) noexcept
{
  worker_ = move(rhs.worker_);
  filterAction_ = move(rhs.filterAction_);
  returnCode_ = move(rhs.returnCode_);
  // Note: A WaitingTaskList cannot be moved,
  // but it only happens at system startup,
  // so oh well.
  waitingTasks_.reset();
  counts_visited_.store(rhs.counts_visited_.load());
  counts_passed_.store(rhs.counts_passed_.load());
  counts_failed_.store(rhs.counts_failed_.load());
  counts_thrown_.store(rhs.counts_thrown_.load());
  return *this;
}

Worker*
WorkerInPath::
getWorker() const
{
  return worker_;
}

WorkerInPath::FilterAction
WorkerInPath::
filterAction() const
{
  return filterAction_;
}

// Used only by Path
bool
WorkerInPath::
returnCode(int /*si*/) const
{
  return returnCode_;
}

string const&
WorkerInPath::
label() const
{
  return worker_->label();
}

// Used only by Path
void
WorkerInPath::
clearCounters()
{
  //counts_ = Counts_t{};
  counts_visited_ = 0;
  counts_passed_ = 0;
  counts_failed_ = 0;
  counts_thrown_ = 0;
}

// Used by writeSummary
size_t
WorkerInPath::
timesVisited() const
{
  //return counts_.times<stats::Visited>();
  return counts_visited_;
}

// Used by writeSummary
size_t
WorkerInPath::
timesPassed() const
{
  //return counts_.times<stats::Passed>();
  return counts_passed_;
}

// Used by writeSummary
size_t
WorkerInPath::
timesFailed() const
{
  //return counts_.times<stats::Failed>();
  return counts_failed_;
}

// Used by writeSummary
size_t
WorkerInPath::
timesExcept() const
{
  //return counts_.times<stats::ExceptionThrown>();
  return counts_thrown_;
}

bool
WorkerInPath::
runWorker(Transition trans, Principal& principal, CurrentProcessingContext* cpc)
{
  // Note: We ignore the return code because we do not process events here.
  worker_->doWork(trans, principal, cpc);
  return true;
}

void
WorkerInPath::
runWorker_event_for_endpath(EventPrincipal& ep, int si, CurrentProcessingContext* cpc)
{
  TDEBUG(4) << "-----> Begin WorkerInPath::runWorker_event_for_endpath (" << si << ") ...\n";
  ++counts_visited_;
  try {
    worker_->doWork_event(ep, si, cpc);
  }
  catch (...) {
    ++counts_thrown_;
    TDEBUG(4) << "-----> End   WorkerInPath::runWorker_event_for_endpath (" << si << ") ... because of EXCEPTION\n";
    throw;
  }
  returnCode_ = worker_->returnCode(si);
  TDEBUG(5) << "-----> WorkerInPath::runWorker_event_for_endpath: si: " << si << " raw returnCode_: " << returnCode_ << "\n";
  if (filterAction_ == Veto) {
    returnCode_ = !returnCode_;
  }
  else if (filterAction_ == Ignore) {
    returnCode_ = true;
  }
  TDEBUG(5) << "-----> WorkerInPath::runWorker_event_for_endpath: si: " << si << " final returnCode_: " << returnCode_ << "\n";
  if (returnCode_) {
    ++counts_passed_;
  }
  else {
    ++counts_failed_;
  }
  TDEBUG(4) << "-----> End   WorkerInPath::runWorker_event_for_endpath (" << si << ") ...\n";
}

void
WorkerInPath::
runWorker_event(WaitingTask* workerDoneTask, EventPrincipal& ep, int si, CurrentProcessingContext* cpc)
{
  TDEBUG(4) << "-----> Begin WorkerInPath::runWorker_event (" << si << ") ...\n";
  // Reset the waiting task list so that it stops running tasks and switches to holding them.
  // Note: There will only ever be one entry in this list!
  {
    ostringstream buf;
    buf << "-----> WorkerInPath::runWorker_event: 0x" << hex << ((unsigned long)this) << dec << " Resetting waitingTasks_ (" << si << ") ...\n";
    TDEBUG(6) << buf.str();
  }
  waitingTasks_.reset();
  waitingTasks_.add(workerDoneTask);
  ++counts_visited_;
  auto workerInPathDoneFunctor = [this, workerDoneTask, &ep, si, cpc](exception_ptr const* ex) mutable {
    TDEBUG(4) << "=====> Begin workerInPathDoneTask (" << si << ") ...\n";
    if (ex != nullptr) {
      ++counts_thrown_;
      waitingTasks_.doneWaiting(*ex);
      TDEBUG(4) << "=====> End   workerInPathDoneTask (" << si << ") ... because of exception\n";
      return;
    }
    returnCode_ = worker_->returnCode(si);
    TDEBUG(5) << "=====> workerInPathDoneTask: si: " << si << " raw returnCode_: " << returnCode_ << "\n";
    if (filterAction_ == Veto) {
      returnCode_ = !returnCode_;
    }
    else if (filterAction_ == Ignore) {
      returnCode_ = true;
    }
    TDEBUG(5) << "=====> workerInPathDoneTask: si: " << si << " final returnCode_: " << returnCode_ << "\n";
    if (returnCode_) {
      ++counts_passed_;
    }
    else {
      ++counts_failed_;
    }
    waitingTasks_.doneWaiting(exception_ptr{});
    TDEBUG(4) << "=====> End   workerInPathDoneTask (" << si << ") ... returnCode_: " << returnCode_ << "\n";
  };
  auto workerInPathDoneTask = make_waiting_task(tbb::task::allocate_root(), workerInPathDoneFunctor);
  try {
    worker_->doWork_event(workerInPathDoneTask, ep, si, cpc);
  }
  catch (...) {
    ++counts_thrown_;
    waitingTasks_.doneWaiting(current_exception());
    TDEBUG(4) << "-----> End   WorkerInPath::runWorker_event (" << si << ") ... because of exception\n";
    return;
  }
  TDEBUG(4) << "-----> End   WorkerInPath::runWorker_event (" << si << ") ...\n";
}

} // namespace art

