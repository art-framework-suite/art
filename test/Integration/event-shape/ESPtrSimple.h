#ifndef test_Integration_event_shape_ESPtrSimple_h
#define test_Integration_event_shape_ESPtrSimple_h

#include "art/Persistency/Common/Ptr.h"
#include "test/TestObjects/ToyProducts.h"

namespace arttest {
  struct ESPtrSimple;
}

struct arttest::ESPtrSimple {
  art::Ptr<Simple> p;
};

#endif /* test_Integration_event_shape_ESPtrSimple_h */

// Local Variables:
// mode: c++
// End:
