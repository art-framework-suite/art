#ifndef LQOOOOAS
#define LQOOOOAS

#include "cetlib/exempt_ptr.h"
#include "tech-testbed/Schedule.hh"
#include "tech-testbed/SerialTaskQueue.h"

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

#endif
