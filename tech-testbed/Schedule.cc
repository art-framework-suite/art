#include "tech-testbed/Schedule.hh"

demo::Schedule::
Schedule(ScheduleID sID)
:
  sID_(sID),
  eventsProcessed_(0)
{
}

demo::ScheduleID
demo::Schedule::
id() const
{
  return sID_;
}

size_t
demo::Schedule::
eventsProcessed() const
{
  return eventsProcessed_;
}

void
demo::Schedule::
operator()(std::unique_ptr<EventPrincipal> &&)
{
  ++eventsProcessed_;
}
