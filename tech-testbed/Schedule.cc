#include "tech-testbed/Schedule.hh"

#include <algorithm>
#include <ostream>

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
  timeTaken_(0.0),
  prevCycle_(tbb::tick_count::now()),
  eventTimes_(),
  ids_(),
  cycleTimes_()
{
  eventTimes_.reserve(4000);
  ids_.reserve(4000);
  cycleTimes_.reserve(4000);
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
operator()(std::unique_ptr<EventPrincipal> && ep)
{
  tbb::tick_count t0 { tbb::tick_count::now() };
  std::vector<double> results(dist_(engine_));
  std::generate_n(results.begin(),
                  results.size(),
                  [this]() { return dist_(engine_); });
  tbb::tick_count t1 { tbb::tick_count::now() };
  ++eventsProcessed_;
  items_ += results.size();
  eventTimes_.emplace_back(t1 -t0);
  timeTaken_ += eventTimes_.back();
  ids_.emplace_back(ep->id);
  cycleTimes_.emplace_back(t1 - prevCycle_);
  // Factor out the measurement generation time.
  prevCycle_ = tbb::tick_count::now();
}

void demo::Schedule::printHeader(std::ostream & os)
{
  os << "sid proc eid cycle\n";
}

void demo::Schedule::print(std::ostream & os) const
{
  for (size_t i = 0, e = eventTimes_.size(); i != e; ++i) {
    os << sID_
       << " "
       << eventTimes_[i].seconds()
       << " "
       << ids_[i]
       << " "
       << cycleTimes_[i].seconds()
       << "\n";
  }
}

std::ostream& operator << (std::ostream & os, demo::Schedule const & s)
{
  s.print(os);
  return os;
}
