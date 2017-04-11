// ======================================================================
//
// ServiceRegistry
//
// ======================================================================

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"

#include "boost/thread/tss.hpp"

using fhicl::ParameterSet;
using art::ServiceRegistry;
using art::ServiceToken;

ServiceToken
ServiceRegistry::setContext(ServiceToken const& iNewToken)
{
  ServiceToken result {manager_};
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
  auto result = std::make_shared<ServicesManager>(iPS,
                                                  ServiceRegistry::instance().lm_,
                                                  reg);
  return ServiceToken{result};
}

ServiceRegistry&
ServiceRegistry::instance()
{
  static boost::thread_specific_ptr<ServiceRegistry> s_registry;
  if (nullptr == s_registry.get()) {
    s_registry.reset(new ServiceRegistry);
  }
  return *s_registry;
}

// ======================================================================
