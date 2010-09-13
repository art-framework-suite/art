//
// Package:     ServiceRegistry
// Class  :     ServiceRegistry


#include "boost/thread/tss.hpp"

#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/ParameterSet/MakeParameterSets.h"

using fhicl::ParameterSet;


//
// constructors and destructor
//
edm::ServiceRegistry::ServiceRegistry()
{ }

edm::ServiceRegistry::~ServiceRegistry()
{ }

//
// member functions
//
edm::ServiceToken
edm::ServiceRegistry::setContext(const edm::ServiceToken& iNewToken)
{
   edm::ServiceToken returnValue(manager_);
   manager_ = iNewToken.manager_;
   return returnValue;
}

void
edm::ServiceRegistry::unsetContext(const ServiceToken& iOldToken)
{
   manager_ = iOldToken.manager_;
}

//
// const member functions
//
edm::ServiceToken
edm::ServiceRegistry::presentToken() const
{
   return manager_;
}

//
// static member functions
//

edm::ServiceToken
edm::ServiceRegistry::createServicesFromConfig(std::string const& config) {
   boost::shared_ptr<std::vector<ParameterSet> > pServiceSets;
   boost::shared_ptr<ParameterSet> params;
   edm::makeParameterSets(config, params, pServiceSets);

   //create the services
   return ServiceToken(edm::ServiceRegistry::createSet(*pServiceSets.get()));
}

edm::ServiceToken
edm::ServiceRegistry::createSet(const std::vector<ParameterSet>& iPS)
{
   using namespace edm::serviceregistry;
   boost::shared_ptr<ServicesManager> returnValue(new ServicesManager(iPS));
   return edm::ServiceToken(returnValue);
}
edm::ServiceToken
edm::ServiceRegistry::createSet(const std::vector<ParameterSet>& iPS,
                                ServiceToken iToken,
                                serviceregistry::ServiceLegacy iLegacy)
{
   using namespace edm::serviceregistry;
   boost::shared_ptr<ServicesManager> returnValue(new ServicesManager(iToken,iLegacy,iPS));
   return edm::ServiceToken(returnValue);
}

edm::ServiceRegistry&
edm::ServiceRegistry::instance()
{
   static boost::thread_specific_ptr<ServiceRegistry> s_registry;
   if(0 == s_registry.get()){
      s_registry.reset(new ServiceRegistry);
   }
   return *s_registry;
}
