// ======================================================================
//
// MyService
//
// ======================================================================

#include "MyServiceInterface.h"
#include "art/Framework/Services/Registry/ServiceDefinitionMacros.h"

namespace {
  class MySharedService : public art::test::MyServiceInterface {
  public:
    explicit MySharedService(fhicl::ParameterSet const&) noexcept {}
  };
}

DECLARE_ART_SERVICE_INTERFACE_IMPL(MySharedService,
                                   art::test::MyServiceInterface,
                                   SHARED)
DEFINE_ART_SERVICE_INTERFACE_IMPL(MySharedService,
                                  art::test::MyServiceInterface)
