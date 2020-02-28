#include "art/Framework/Services/Optional/TrivialFileDelivery.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/ParameterSet.h"

namespace arttest {
  class ServiceUsingIface;
}

class arttest::ServiceUsingIface {
public:
  ServiceUsingIface(fhicl::ParameterSet const&);
};

arttest::ServiceUsingIface::ServiceUsingIface(fhicl::ParameterSet const&)
{
  art::ServiceHandle<art::TrivialFileDelivery> h [[maybe_unused]];
}

// The DECLARE macro call should be moved to the header file, should you
// create one.
DECLARE_ART_SERVICE(arttest::ServiceUsingIface, LEGACY)
DEFINE_ART_SERVICE(arttest::ServiceUsingIface)
