// Service to make sure we can use another service.

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "test/Integration/ServiceUsing.h"

void
arttest::ServiceUsing::
maybeGetNewValue(std::string const & service_name)
{
  static std::string const watched_service("Reconfigurable");
  if (service_name == watched_service) {
    cached_debug_value_ = getNewValue();
  }
}

DEFINE_ART_SERVICE(arttest::ServiceUsing)
