#ifndef test_Framework_Services_Optional_MyService_h
#define test_Framework_Services_Optional_MyService_h

// MyService: test service inheriting from MyServiceInterface.

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "test/Framework/Services/Interfaces/MyServiceInterface.h"

namespace arttest {
  class MyService;
}

namespace art {
  class ActivityRegistry;
}

namespace fhicl {
  class ParameterSet;
}

// ----------------------------------------------------------------------

class arttest::MyService : public arttest::MyServiceInterface
{
public:
  MyService(fhicl::ParameterSet const&, art::ActivityRegistry&);

  // Use compiler-generated copy c'tor, copy assignment, and d'tor

private:

};  // MyService

// ======================================================================

DECLARE_ART_SERVICE_INTERFACE_IMPL(arttest::MyService, arttest::MyServiceInterface, LEGACY)
#endif /* test_Framework_Services_Optional_MyService_h */

// Local Variables:
// mode: c++
// End:
