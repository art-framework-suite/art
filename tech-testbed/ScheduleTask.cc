#include "tech-testbed/ScheduleTask.hh"

#include "tech-testbed/make_reader.hh"

demo::ScheduleTask::
ScheduleTask(std::unique_ptr<EventPrincipal> && ep,
             cet::exempt_ptr<Schedule> const sched,
             task * topTask,
             SerialTaskQueue & sQ,
             size_t & evCounter)
 :
  ep_(std::move(ep)),
  sched_(sched),
  topTask_(topTask),
  sQ_(sQ),
  evCounter_(evCounter)
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
                          evCounter_);
  sQ_.push(std::move(func));
  return nullptr;
}
