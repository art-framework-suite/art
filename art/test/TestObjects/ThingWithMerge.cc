
//

#include "art/test/TestObjects/ThingWithMerge.h"

namespace arttest {

  bool ThingWithMerge::mergeProduct(ThingWithMerge const& newThing) {
    a += newThing.a;
    return true;
  }
}
