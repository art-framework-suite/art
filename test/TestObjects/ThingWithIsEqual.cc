
//

#include "test/TestObjects/ThingWithIsEqual.h"

namespace arttest {

  bool ThingWithIsEqual::isProductEqual(ThingWithIsEqual const& newThing) const {
    return a == newThing.a;
  }
}
