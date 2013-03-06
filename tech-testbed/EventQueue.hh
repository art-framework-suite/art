#ifndef tech_testbed_EventQueue_hh
#define tech_testbed_EventQueue_hh

#include "tech-testbed/EventPrincipal.hh"

#include "tbb/concurrent_queue.h"
#include <memory>

namespace demo {
  typedef tbb::concurrent_bounded_queue<std::shared_ptr<demo::EventPrincipal> > EventQueue;
}

#endif /* tech_testbed_EventQueue_hh */
