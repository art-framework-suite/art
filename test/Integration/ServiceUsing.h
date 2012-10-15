#ifndef test_Integration_ServiceUsing_h
#define test_Integration_ServiceUsing_h

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/ParameterSet.h"

#include "test/Integration/Reconfigurable.h"

namespace arttest {
  class ServiceUsing;
}

class arttest::ServiceUsing {
public:
  ServiceUsing(fhicl::ParameterSet const &, art::ActivityRegistry &);

  int getCachedValue() const;
private:
  void maybeGetNewValue(std::string const & service_name);

  int getNewValue();

  int cached_debug_value_;
};

inline
int
arttest::ServiceUsing::
getCachedValue() const
{
  return cached_debug_value_;
}

inline
int
arttest::ServiceUsing::
getNewValue()
{
  return art::ServiceHandle<Reconfigurable>()->get_debug_level();
}

DECLARE_ART_SERVICE(arttest::ServiceUsing, LEGACY)
#endif /* test_Integration_ServiceUsing_h */

// Local Variables:
// mode: c++
// End:
