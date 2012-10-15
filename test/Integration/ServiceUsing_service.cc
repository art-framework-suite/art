// Service to make sure we can use another service.

#include "test/Integration/ServiceUsing.h"
#include "test/Integration/Wanted.h"

arttest::ServiceUsing::
ServiceUsing(fhicl::ParameterSet const &, art::ActivityRegistry & reg )
  :
  cached_debug_value_(getNewValue()) // Uses ServiceHandle<Reconfigurable>
{
  (void) art::ServiceHandle<Wanted>(); // Required to force construction.
  reg.watchPostServiceReconfigure(this, &arttest::ServiceUsing::maybeGetNewValue);
}

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
