#ifndef art_Framework_Services_Registry_ServiceToken_h
#define art_Framework_Services_Registry_ServiceToken_h

// ======================================================================
//
// ServiceToken - Token used to denote a 'service set'
//
// When you request a new 'service set' to be created from the
// ServiceRegistry, the ServiceRegistry will return a ServiceToken.  When
// you want this 'service set' to be used, create a
// ServiceRegistry::Operate by passing the ServiceToken via the constructor.
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceWrapper.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "cpp0x/memory"

namespace art {
  class ServiceRegistry;
  class ActivityRegistry;

  class ServiceToken;
}

// ----------------------------------------------------------------------

class art::ServiceToken
{
  friend class ServiceRegistry;
  friend class ServicesManager;

public:
  ServiceToken( ) { }

  template< class T >
    bool add( std::unique_ptr<T> && serv )
  {
     return manager_->put(std::shared_ptr<ServiceWrapper<T> >(new ServiceWrapper<T>(serv)));
  }

  void forceCreation()
  {
    manager_->forceCreation();
  }

  void getParameterSets(ServicesManager::ParameterSets& out) const
  { manager_->getParameterSets(out); }
  void putParameterSets(ServicesManager::ParameterSets const& in)
  { manager_->putParameterSets(in); }

private:
  ServiceToken( std::shared_ptr<ServicesManager> iManager )
  : manager_( iManager )
  { }

  std::shared_ptr<ServicesManager> manager_;

};  // ServiceToken

// ======================================================================

#endif /* art_Framework_Services_Registry_ServiceToken_h */

// Local Variables:
// mode: c++
// End:
