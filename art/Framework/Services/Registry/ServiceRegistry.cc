// ======================================================================
//
// ServiceRegistry
//
// ======================================================================

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"

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
ServiceRegistry::createSet(ParameterSets const & iPS, ActivityRegistry & reg)
{
  std::shared_ptr<ServicesManager>
    result(new ServicesManager(iPS,
                               ServiceRegistry::instance().lm_,
                               reg)
          );
  return ServiceToken(result);
}

ServiceRegistry &
  ServiceRegistry::instance()
{
  static ServiceRegistry s_reg; // Thread-safe per C++2011.
  return s_reg;
}

// ======================================================================
