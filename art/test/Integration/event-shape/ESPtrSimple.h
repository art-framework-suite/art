#ifndef test_Integration_event_shape_ESPtrSimple_h
#define test_Integration_event_shape_ESPtrSimple_h

#include "canvas/Persistency/Common/Ptr.h"
#include "art/test/TestObjects/ToyProducts.h"

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
