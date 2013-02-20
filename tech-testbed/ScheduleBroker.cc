#include "tech-testbed/ScheduleBroker.hh"

#include <cassert>
#include <iostream>
#include <sstream>

demo::ScheduleBroker::
ScheduleBroker(EventPrincipalQueue & epQ,
               ScheduleQueue & sQ,
               tbb::task * topTask,
               WaitingTaskList & pTasks)
:
  draining_(false),
  epQ_(epQ),
  sQ_(sQ),
  topTask_(topTask),
  pTasks_(pTasks)
{
  assert(topTask_ != nullptr);
}

void
demo::ScheduleBroker::
drain()
{
  draining_.store(true);
}

tbb::task *
demo::ScheduleBroker::
execute() {
  EventPrincipalQueue::value_type ep;
  ScheduleQueue::value_type sched;
  std::ostringstream os;
  os << "draining_: " << draining_
     << ", epQ_.size(): " << epQ_.size()
     << "\n";
  std::cerr << os.str();
  while (!(draining_ && epQ_.empty())) {
    // Prevent hang if set to drain *after* we emptied the queue.
    if (epQ_.try_pop(ep)) {
      sQ_.pop(sched);
      pTasks_.add(new (allocate_root()) ScheduleTask { ep, sched, sQ_ });
    } else {
      usleep(10); // Don't spin too fast.
    }
  }
  return nullptr;
}
