#ifndef art_test_Integration_event_shape_ESPtrSimple_h
#define art_test_Integration_event_shape_ESPtrSimple_h

#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Persistency/Common/Ptr.h"

namespace arttest {
  struct ESPtrSimple;
}

struct arttest::ESPtrSimple {
  art::Ptr<Simple> p;
  void
  aggregate(ESPtrSimple const&) const
  {}
};

#endif /* art_test_Integration_event_shape_ESPtrSimple_h */

// Local Variables:
// mode: c++
// End:
