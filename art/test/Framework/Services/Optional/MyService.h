#ifndef art_test_Framework_Services_Optional_MyService_h
#define art_test_Framework_Services_Optional_MyService_h

// MyService: test service inheriting from MyServiceInterface.

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/test/Framework/Services/Interfaces/MyServiceInterface.h"
#include "fhiclcpp/fwd.h"

namespace art::test {
  class MyService : public MyServiceInterface {
  public:
    MyService(fhicl::ParameterSet const&);
  };
}

DECLARE_ART_SERVICE_INTERFACE_IMPL(art::test::MyService,
                                   art::test::MyServiceInterface,
                                   LEGACY)
#endif /* art_test_Framework_Services_Optional_MyService_h */

// Local Variables:
// mode: c++
// End:
