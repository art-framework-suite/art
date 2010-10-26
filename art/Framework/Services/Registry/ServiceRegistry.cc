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
art::ServiceRegistry::ServiceRegistry()
{ }

art::ServiceRegistry::~ServiceRegistry()
{ }

//
// member functions
//
art::ServiceToken
art::ServiceRegistry::setContext(const art::ServiceToken& iNewToken)
{
   art::ServiceToken returnValue(manager_);
   manager_ = iNewToken.manager_;
   return returnValue;
}

void
art::ServiceRegistry::unsetContext(const ServiceToken& iOldToken)
{
   manager_ = iOldToken.manager_;
}

//
// const member functions
//
art::ServiceToken
art::ServiceRegistry::presentToken() const
{
   return manager_;
}

//
// static member functions
//

art::ServiceToken
art::ServiceRegistry::createServicesFromConfig(std::string const& config) {
   boost::shared_ptr<std::vector<ParameterSet> > pServiceSets;
   boost::shared_ptr<ParameterSet> params;
   art::makeParameterSets(config, params, pServiceSets);

   //create the services
   return ServiceToken(art::ServiceRegistry::createSet(*pServiceSets.get()));
}

art::ServiceToken
art::ServiceRegistry::createSet(const std::vector<ParameterSet>& iPS)
{
   using namespace art::serviceregistry;
   boost::shared_ptr<ServicesManager> returnValue(new ServicesManager(iPS));
   return art::ServiceToken(returnValue);
}
art::ServiceToken
art::ServiceRegistry::createSet(const std::vector<ParameterSet>& iPS,
                                ServiceToken iToken,
                                serviceregistry::ServiceLegacy iLegacy)
{
   using namespace art::serviceregistry;
   boost::shared_ptr<ServicesManager> returnValue(new ServicesManager(iToken,iLegacy,iPS));
   return art::ServiceToken(returnValue);
}

art::ServiceRegistry&
art::ServiceRegistry::instance()
{
   static boost::thread_specific_ptr<ServiceRegistry> s_registry;
   if(0 == s_registry.get()){
      s_registry.reset(new ServiceRegistry);
   }
   return *s_registry;
}
