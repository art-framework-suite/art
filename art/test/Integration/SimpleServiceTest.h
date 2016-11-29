#ifndef art_test_Integration_SimpleServiceTest_h
#define art_test_Integration_SimpleServiceTest_h
////////////////////////////////////////////////////////////////////////
// Class:       SimpleServiceTest
// Plugin Type: service (art v1_19_00_rc3)
// File:        SimpleServiceTest.h
//
// Generated at Mon May  9 16:37:35 2016 by Christopher Green using cetskelgen
// from cetlib version v1_17_04.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Services/Registry/ServiceMacros.h"

namespace fhicl {
  class ParameterSet;
}

namespace arttest {
  class SimpleServiceTest;
}


class arttest::SimpleServiceTest {
public:
  explicit SimpleServiceTest(fhicl::ParameterSet const &);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  bool verifyStatus() const;
private:
};

DECLARE_ART_SERVICE(arttest::SimpleServiceTest, LEGACY)
#endif /* art_test_Integration_SimpleServiceTest_h */

// Local Variables:
// mode: c++
// End:
