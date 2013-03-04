#ifndef tech_testbed_make_reader_hh
#define tech_testbed_make_reader_hh

#include "cetlib/exempt_ptr.h"
#include "tech-testbed/Schedule.hh"
#include "tech-testbed/SerialTaskQueue.hh"

#include "tbb/task.h"

#include <cstdlib>
#include <functional>

namespace demo {
  std::function<void ()>
  make_reader(cet::exempt_ptr<Schedule> sched,
              tbb::task * topTask,
              SerialTaskQueue & sQ,
              size_t & evCounter);
}

#endif /* tech_testbed_make_reader_hh */
