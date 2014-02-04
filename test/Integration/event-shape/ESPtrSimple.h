#include "art/Persistency/Common/Ptr.h"
#include "test/TestObjects/ToyProducts.h"

namespace arttest {
  struct ESPtrSimple;
}

struct arttest::ESPtrSimple {
  art::Ptr<Simple> p;
};
