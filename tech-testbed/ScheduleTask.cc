#include "tech-testbed/ScheduleTask.hh"

demo::ScheduleTask::
ScheduleTask(cet::exempt_ptr<EventPrincipal> const & ep,
             cet::exempt_ptr<Schedule> const & sched,
             ScheduleQueue & sQ)
 :
  ep_(ep),
  sched_(sched),
  sQ_(sQ)
{
}

tbb::task *
demo::ScheduleTask::
execute()
{
  // Do the work.
  (*sched_)(ep_);
  // Make our schedule available for more work.
  sQ_.push(sched_);
  // If we happen to end up with another ScheduleTask operating on this
  // schedule before this one has gone away this is perfectly fine:
  // schedule is still ready to do the work.
  return nullptr;
}
