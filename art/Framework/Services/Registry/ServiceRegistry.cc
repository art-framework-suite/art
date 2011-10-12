// ======================================================================
//
// ServiceRegistry
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceRegistry.h"

#include "boost/thread/tss.hpp"

using fhicl::ParameterSet;
using art::ServiceRegistry;
using art::ServiceToken;

ServiceRegistry::ServiceRegistry()
: lm_( "service" )
{ }

ServiceRegistry::~ServiceRegistry()
{ }

ServiceToken
  ServiceRegistry::setContext( ServiceToken const & iNewToken )
{
  ServiceToken result(manager_);
  manager_ = iNewToken.manager_;
  return result;
}

void
  ServiceRegistry::unsetContext( ServiceToken const & iOldToken )
{
  manager_ = iOldToken.manager_;
}

ServiceToken
  ServiceRegistry::presentToken() const
{
  return manager_;
}

ServiceToken
  ServiceRegistry::createSet( std::vector<ParameterSet> const & iPS )
{
  std::shared_ptr<ServicesManager>
    result( new ServicesManager( iPS
                               , ServiceRegistry::instance().lm_
          )                    );
  return ServiceToken(result);
}
ServiceToken
  ServiceRegistry::createSet( std::vector<ParameterSet> const & iPS
                            , ServiceToken iToken
                            , ServiceLegacy iLegacy
                            )
{
  std::shared_ptr<ServicesManager>
    result( new ServicesManager( iToken
                               , iLegacy
                               , iPS
                               , ServiceRegistry::instance().lm_
          )                    );
  return ServiceToken(result);
}

ServiceRegistry &
  ServiceRegistry::instance()
{
  static boost::thread_specific_ptr<ServiceRegistry> s_registry;
  if( 0 == s_registry.get() ) {
    s_registry.reset(new ServiceRegistry);
  }
  return *s_registry;
}

// ======================================================================
