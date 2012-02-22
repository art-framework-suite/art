// Service to make sure we can use another service.

#include "art/Framework/Services/System/CurrentModule.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

namespace arttest {
  class ServiceUsing;
}

class arttest::ServiceUsing {
public:
  ServiceUsing(fhicl::ParameterSet const &, art::ActivityRegistry &);
};

arttest::ServiceUsing::
ServiceUsing(fhicl::ParameterSet const &, art::ActivityRegistry &)
{
  art::ServiceHandle<art::CurrentModule>();
}

DEFINE_ART_SERVICE(arttest::ServiceUsing)
