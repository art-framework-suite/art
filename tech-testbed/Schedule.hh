#ifndef tech_testbed_Schedule_hh
#define tech_testbed_Schedule_hh

#include "cetlib/exempt_ptr.h"
#include "tech-testbed/EventPrincipal.hh"

#include <cstdint>

namespace demo {
  class Schedule;

  typedef uint8_t ScheduleID;
}

class demo::Schedule {
public:
  Schedule(ScheduleID sID);

  ScheduleID id() const;
  size_t eventsProcessed() const;

  void operator()(cet::exempt_ptr<EventPrincipal> const & ep);

private:
  ScheduleID const sID_;
  size_t eventsProcessed_; // Note: not thread-safe.
};
#endif /* tech_testbed_Schedule_hh */
