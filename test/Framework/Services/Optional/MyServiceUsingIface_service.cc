#include "art/Framework/Services/Optional/TrivialFileDelivery.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/ParameterSet.h"

namespace arttest {
  class ServiceUsingIface;
}

class arttest::ServiceUsingIface {
public:
  ServiceUsingIface(fhicl::ParameterSet const &,
                    art::ActivityRegistry &);
};

arttest::ServiceUsingIface::
ServiceUsingIface(fhicl::ParameterSet const &,
                    art::ActivityRegistry &) {
  art::ServiceHandle<art::TrivialFileDelivery> h __attribute__((unused));
}

DEFINE_ART_SERVICE(arttest::ServiceUsingIface)
