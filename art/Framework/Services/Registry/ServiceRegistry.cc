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
ServiceRegistry::setContext(const ServiceToken& iNewToken)
{
  ServiceToken returnValue(manager_);
  manager_ = iNewToken.manager_;
  return returnValue;
}

void
ServiceRegistry::unsetContext(const ServiceToken& iOldToken)
{
  manager_ = iOldToken.manager_;
}

ServiceToken
ServiceRegistry::presentToken() const
{
  return manager_;
}

#if 0
ServiceToken
ServiceRegistry::createSet(const std::vector<ParameterSet>& iPS)
{
  boost::shared_ptr<ServicesManager> returnValue(new ServicesManager(iPS,lm_));
  return ServiceToken(returnValue);
}
ServiceToken
ServiceRegistry::createSet(const std::vector<ParameterSet>& iPS,
                            ServiceToken iToken,
                            ServiceLegacy iLegacy)
{
  boost::shared_ptr<ServicesManager> returnValue(new ServicesManager(iToken,iLegacy,iPS,lm_));
  return ServiceToken(returnValue);
}
#endif  // 0

ServiceRegistry&
ServiceRegistry::instance()
{
  static boost::thread_specific_ptr<ServiceRegistry> s_registry;
  if(0 == s_registry.get()){
    s_registry.reset(new ServiceRegistry);
  }
  return *s_registry;
}

// ======================================================================
