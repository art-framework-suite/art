#ifndef art_Framework_EventProcessor_ServiceDirector_h
#define art_Framework_EventProcessor_ServiceDirector_h
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "fhiclcpp/ParameterSet.h"

namespace art {
  class ServiceDirector;
}

class art::ServiceDirector {
public:
  ServiceDirector(fhicl::ParameterSet const & pset,
                  ActivityRegistry & areg,
                  ServiceToken & token);
  template <typename SERVICE>
    void addSystemService(std::unique_ptr<SERVICE> && servicePtr);
private:
  ServiceToken & serviceToken_;
};

#ifndef __GCCXML__
template <typename SERVICE>
void
art::ServiceDirector::
addSystemService(std::unique_ptr<SERVICE> && servicePtr)
{
  serviceToken_.add(std::move(servicePtr));
}
#endif

#endif /* art_Framework_EventProcessor_ServiceDirector_h */

// Local Variables:
// mode: c++
// End:
