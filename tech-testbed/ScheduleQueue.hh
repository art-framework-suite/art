#ifndef tech_testbed_ScheduleQueue_hh
#define tech_testbed_ScheduleQueue_hh

#include "cetlib/exempt_ptr.h"
#include "tech-testbed/Schedule.hh"

#include "tbb/concurrent_queue.h"

namespace demo {

  typedef tbb::concurrent_bounded_queue<cet::exempt_ptr<Schedule> > ScheduleQueue;

}
#endif /* tech_testbed_ScheduleQueue_hh */

// Local Variables:
// mode: c++
// End:
