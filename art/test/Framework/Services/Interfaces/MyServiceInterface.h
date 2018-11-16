#ifndef art_test_Framework_Services_Interfaces_MyServiceInterface_h
#define art_test_Framework_Services_Interfaces_MyServiceInterface_h

// MyServiceInterface: interface from which a test service can inherit.

#include "art/Framework/Services/Registry/ServiceMacros.h"

namespace arttest {
  class MyServiceInterface {};
}

DECLARE_ART_SERVICE_INTERFACE(arttest::MyServiceInterface, LEGACY)
#endif /* art_test_Framework_Services_Interfaces_MyServiceInterface_h */

// Local Variables:
// mode: c++
// End:
