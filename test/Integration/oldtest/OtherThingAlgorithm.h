#ifndef test_Integration_oldtest_OtherThingAlgorithm_h
#define test_Integration_oldtest_OtherThingAlgorithm_h

#include <string>
#include "test/TestObjects/OtherThingCollectionfwd.h"
#include "art/Framework/Core/Frameworkfwd.h"

namespace arttest {

  class OtherThingAlgorithm {
  public:
    OtherThingAlgorithm() : theDebugLevel(0) {}

    /// Runs the algorithm and returns a list of OtherThings
    /// The user declares the vector and calls this method.
    void run(art::Event const& ev,
	     OtherThingCollection& otherThingCollection,
	     std::string const& thingLabel = std::string("Thing"),
	     std::string const& instance = std::string(), bool refsAreTransient = false);

  private:
    int    theDebugLevel;
  };

}

#endif /* test_Integration_oldtest_OtherThingAlgorithm_h */

// Local Variables:
// mode: c++
// End:
