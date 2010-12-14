#ifndef Integration_ThingAlgorithm_h
#define Integration_ThingAlgorithm_h

/** \class ThingAlgorithm
 *
 ************************************************************/
#include "test/TestObjects/ThingCollectionfwd.h"

namespace arttest {
  class ThingAlgorithm {
  public:
    ThingAlgorithm(long iOffsetDelta = 0) : theDebugLevel(0), offset(0), offsetDelta(iOffsetDelta) {}


    /// Runs the algorithm and returns a list of Things
    /// The user declares the vector and calls this method.
    void run(ThingCollection& thingCollection);
  private:
    int    theDebugLevel;
    long   offset;
    long   offsetDelta;
  };

}

#endif
