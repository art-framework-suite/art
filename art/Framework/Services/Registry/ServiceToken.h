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
#include "boost/shared_ptr.hpp"

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

  // the argument's signals are propagated to the Service's held by the token
  void connectTo( ActivityRegistry & );
  // the argument's signals will forward the token's signals
  void connect( ActivityRegistry & );

  // copy our Service's slots to the argument's signals
  void copySlotsTo( ActivityRegistry & );
  // the copy the argument's slots to the token's signals
  void copySlotsFrom( ActivityRegistry & );

  template< class T >
    bool add( std::auto_ptr<T> serv )
  {
     return manager_->put(boost::shared_ptr<ServiceWrapper<T> >(new ServiceWrapper<T>(serv)));
  }

  void forceCreation(ActivityRegistry& reg)
  {
    manager_->forceCreation(reg);
  }

  void getParameterSets(ServicesManager::ParameterSets& out) const
  { manager_->getParameterSets(out); }
  void putParameterSets(ServicesManager::ParameterSets const& in)
  { manager_->putParameterSets(in); }

private:
  ServiceToken( boost::shared_ptr<ServicesManager> iManager )
  : manager_( iManager )
  { }

  boost::shared_ptr<ServicesManager> manager_;

};  // ServiceToken

// ======================================================================

#endif /* art_Framework_Services_Registry_ServiceToken_h */

// Local Variables:
// mode: c++
// End:
