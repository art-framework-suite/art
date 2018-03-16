// ======================================================================
//
// ServiceRegistry
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"

using art::ServiceRegistry;
using art::ServiceToken;
using fhicl::ParameterSet;

ServiceToken
ServiceRegistry::setContext(ServiceToken const& iNewToken)
{
  ServiceToken result{manager_};
  manager_ = iNewToken.manager_;
  return result;
}

void
ServiceRegistry::unsetContext(ServiceToken const& iOldToken)
{
  manager_ = iOldToken.manager_;
}

ServiceToken
ServiceRegistry::presentToken() const
{
  return ServiceToken{manager_};
}

ServiceToken
ServiceRegistry::createSet(ParameterSets const& iPS, ActivityRegistry& reg)
{
  auto result = std::make_shared<ServicesManager>(
    iPS, ServiceRegistry::instance().lm_, reg);
  return ServiceToken{result};
}

ServiceRegistry&
ServiceRegistry::instance()
{
  static ServiceRegistry me;
  return me;
}

// ======================================================================
