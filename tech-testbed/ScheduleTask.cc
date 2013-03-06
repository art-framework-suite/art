#include "tech-testbed/ScheduleTask.hh"

#include "tech-testbed/make_reader.hh"

demo::ScheduleTask::
ScheduleTask(std::shared_ptr<EventPrincipal> && ep,
             cet::exempt_ptr<Schedule> const sched,
             task * topTask,
             SerialTaskQueue & sQ,
             EventQueue & eQ)
 :
  ep_(std::move(ep)),
  sched_(sched),
  topTask_(topTask),
  sQ_(sQ),
  eQ_(eQ)
{
}

tbb::task *
demo::ScheduleTask::
execute()
{
  // Do the work.
  (*sched_)(std::move(ep_));
  // Push a new reader for our schedule back onto the queue.
  auto func = make_reader(sched_,
                          topTask_,
                          sQ_,
                          eQ_);
  sQ_.push(std::move(func));
  return nullptr;
}
