#include "MyService.h"
#include "art/Framework/Services/Registry/ServiceDefinitionMacros.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"

namespace arttest {
  class ServiceUsingIface;
}

using art::test::MyService;

class arttest::ServiceUsingIface {
public:
  ServiceUsingIface(fhicl::ParameterSet const&);
};

arttest::ServiceUsingIface::ServiceUsingIface(fhicl::ParameterSet const&)
{
  art::ServiceHandle<MyService> h [[maybe_unused]];
}

// The DECLARE macro call should be moved to the header file, should you
// create one.
DECLARE_ART_SERVICE(arttest::ServiceUsingIface, LEGACY)
DEFINE_ART_SERVICE(arttest::ServiceUsingIface)
