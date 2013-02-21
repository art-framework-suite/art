#include "tech-testbed/Schedule.hh"

#include <algorithm>

demo::Schedule::
Schedule(ScheduleID sID,
         unsigned int seed,
         size_t scale)
:
  sID_(sID),
  engine_(seed),
  dist_(3.0, scale),
  eventsProcessed_(0),
  items_(0),
  timeTaken_(0.0)
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

size_t
demo::Schedule::
itemsGenerated() const
{
  return items_;
}

tbb::tick_count::interval_t
demo::Schedule::
timeTaken() const
{
  return timeTaken_;
}

void
demo::Schedule::
operator()(std::unique_ptr<EventPrincipal> &&)
{
  tbb::tick_count t0 { tbb::tick_count::now() };
  std::vector<double> results(dist_(engine_));
  std::generate_n(results.begin(),
                  results.size(),
                  [this]() { return dist_(engine_); });
  ++eventsProcessed_;
  items_ += results.size();
  timeTaken_ += (tbb::tick_count::now() - t0);
}
