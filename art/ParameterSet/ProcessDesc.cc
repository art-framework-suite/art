/**
   \file
   Implementation of class ProcessDesc

   \author Stefano ARGIRO
   \version
   \date 17 Jun 2005
*/


#include "art/ParameterSet/ProcessDesc.h"

#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/EDMException.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include <iostream>


using namespace cet;
using namespace fhicl;
using namespace std;


namespace art {

  ProcessDesc::ProcessDesc(const ParameterSet & pset)
  : pset_(new ParameterSet(pset)),
    services_(new vector<ParameterSet>())
  {
    setRegistry();
    // cout << pset << endl;
  }

  ProcessDesc::~ProcessDesc()
  { }

  #if 0
  ProcessDesc::ProcessDesc(const string& config)
  : pset_(new ParameterSet),
    services_(new vector<ParameterSet>())
  {
    throw art::Exception(errors::Configuration,"Old config strings no longer accepted");
  }
  #endif  // 0

  void ProcessDesc::setRegistry() const
  {
    // Load every ParameterSet into the Registry
    pset::Registry* reg = pset::Registry::instance();
    pset::loadAllNestedParameterSets(reg, *pset_);
  }


  boost::shared_ptr<ParameterSet>
  ProcessDesc::getProcessPSet() const{
    return pset_;

  }

  boost::shared_ptr<vector<ParameterSet> >
  ProcessDesc::getServicesPSets() const{
    return services_;
  }


  void ProcessDesc::addService(const ParameterSet & pset)
  {
    services_->push_back(pset);
   // Load into the Registry
    pset::Registry* reg = pset::Registry::instance();
    reg->insertMapped(pset);
  }


  void ProcessDesc::addService(const string & service)
  {
    ParameterSet newpset;
    newpset.put("@service_type",service);
    addService(newpset);
  }

  void ProcessDesc::addDefaultService(const string & service)
  {
    typedef vector<ParameterSet>::iterator Iter;
    for(Iter it = services_->begin(), itEnd = services_->end(); it != itEnd; ++it) {
        string name = it->get<string>("@service_type");

        if (name == service) {
          // If the service is already there move it to the end so
          // it will be created before all the others already there
          // This means we use the order from the default services list
          // and the parameters from the configuration file
          while (true) {
            Iter iterNext = it + 1;
            if (iterNext == itEnd) return;
            iter_swap(it, iterNext);
            ++it;
          }
        }
    }
    addService(service);
  }


  void ProcessDesc::addServices(vector<string> const& defaultServices,
                                vector<string> const& forcedServices)
  {
    // Add the forced and default services to services_.
    // In services_, we want the default services first, then the forced
    // services, then the services from the configuration.  It is efficient
    // and convenient to add them in reverse order.  Then after we are done
    // adding, we reverse the vector again to get the desired order.
    reverse(services_->begin(), services_->end());
    for(vector<string>::const_reverse_iterator j = forcedServices.rbegin(),
                                            jEnd = forcedServices.rend();
         j != jEnd; ++j) {
      addService(*j);
    }
    for(vector<string>::const_reverse_iterator i = defaultServices.rbegin(),
                                            iEnd = defaultServices.rend();
         i != iEnd; ++i) {
      addDefaultService(*i);
    }
    reverse(services_->begin(), services_->end());
  }

} // namespace art
