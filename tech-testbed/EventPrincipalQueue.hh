#ifndef tech_testbed_EventPrincipalQueue_hh
#define tech_testbed_EventPrincipalQueue_hh

#include "cetlib/exempt_ptr.h"

#include "tech-testbed/EventPrincipal.hh"

#include "tbb/concurrent_queue.h"

namespace demo {

  typedef tbb::concurrent_bounded_queue<cet::exempt_ptr<EventPrincipal> > EventPrincipalQueue;

}
#endif /* tech_testbed_EventPrincipalQueue_hh */

// Local Variables:
// mode: c++
// End:
