#include "art/Framework/EventProcessor/ServiceDirector.h"

#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/System/CurrentModule.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Framework/Services/System/FloatingPointControl.h"
#include "art/Framework/Services/System/PathSelection.h"
#include "art/Framework/Services/System/ScheduleContext.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "fhiclcpp/ParameterSetRegistry.h"

namespace {
  typedef std::vector<ParameterSet> ParameterSets;

  void
  addService(std::string const & name, ParameterSets & service_set)
  {
    ParameterSet tmp;
    tmp.put("service_type", name);
    fhicl::ParameterSetRegistry::put(tmp);
    service_set.emplace_back(std::move(tmp));
  }

  void
  addOptionalService(std::string const & name,
                     ParameterSet const & source,
                     ParameterSets & service_set)
  {
    ParameterSet pset;
    if (source.get_if_present(name, pset)) {
      service_set.push_back(pset);
    }
  }

  void
  addService(std::string const & name,
             ParameterSet const & source,
             ParameterSets & service_set)
  {
    ParameterSet tmp;
    if (source.get_if_present(name, tmp)) {
      service_set.emplace_back(std::move(tmp));
    } else {
      // Add a suitably simple parameter set.
      addService(name, service_set);
    }
  }

  void extractServices(ParameterSet const & services, ParameterSets & service_set)
  {
    // this is not ideal.  Need to change the ServiceRegistry "createSet" and ServicesManager "put"
    // functions to take the parameter set vector and a list of service objects to be added to
    // the service token.  Alternatively we could get the service token and be allowed to add
    // service objects to it.  Since the servicetoken contains the servicemanager, we might
    // be able to simply add a function to the serviceregistry or servicesmanager that given
    // a service token, it injects a new service object using the "put" of the
    // servicesManager.
    // order might be important here
    // only configured if pset present in services
    addOptionalService("RandomNumberGenerator", services, service_set);
    addOptionalService("SimpleMemoryCheck", services, service_set);
    addOptionalService("Timing", services, service_set);
    addOptionalService("TFileService", services, service_set);
    addService("FileCatalogMetadata", services, service_set);
    ParameterSet user_services = services.get<ParameterSet>("user", ParameterSet());
    std::vector<std::string> keys = user_services.get_pset_keys();
    for (std::vector<std::string>::iterator i = keys.begin(), e = keys.end(); i != e; ++i)
    { addService(*i, user_services, service_set); }
  }
}
#include <iostream>
art::ServiceDirector::
ServiceDirector(fhicl::ParameterSet const & pset,
                ActivityRegistry & areg,
                ServiceToken & serviceToken)
:
  serviceToken_(serviceToken)
{
  fhicl::ParameterSet const services = pset.get<ParameterSet>("services", ParameterSet());
  fhicl::ParameterSet const scheduler = services.get<ParameterSet>("scheduler", ParameterSet());
  bool const wantTracer = scheduler.get<bool>("wantTracer", false);
  // build a list of service parameter sets that will be used by the service registry
  ParameterSets service_set;
  extractServices(services, service_set);
  // configured based on optional parameters
  if (wantTracer) { addService("Tracer", service_set); }
  size_t i = 0;
  for (auto const & ps : service_set) {
    std::cerr << i++ << ": " << ps.to_indented_string() << "\n";
  }
  serviceToken = ServiceRegistry::createSet(service_set, areg);
}
