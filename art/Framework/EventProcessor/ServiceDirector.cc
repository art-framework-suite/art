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

namespace {
  typedef std::vector<ParameterSet> ParameterSets;

  void
  addService(std::string const & name, ParameterSets & service_set)
  {
    service_set.push_back(ParameterSet());
    service_set.back().put("service_type", name);
  }

  void
  addOptionalService(std::string const & name,
                     ParameterSet const & source,
                     ParameterSets & service_set)
  {
    ParameterSet pset;
    if (source.get_if_present(name, pset)) {
      pset.put("service_type", name);
      service_set.push_back(pset);
    }
  }

  void
  addService(std::string const & name,
             ParameterSet const & source,
             ParameterSets & service_set)
  {
    service_set.push_back(source.get<ParameterSet>(name, ParameterSet()));
    service_set.back().put("service_type", name);
  }
}

art::ServiceDirector::
ServiceDirector(fhicl::ParameterSet const & pset,
                ActivityRegistry & areg,
                ServiceToken & serviceToken)
:
  serviceToken_(serviceToken),
  numSchedules_(pset.get<size_t>("services.scheduler.num_schedules", 1))
{
  // Build a list of service parameter sets that will be used by the
  // service registry.
  ParameterSets service_set;
  fhicl::ParameterSet const services =
    pset.get<ParameterSet>("services", ParameterSet());
  extractServices(services, service_set);
  // configured based on optional parameters
  serviceToken =
    ServiceRegistry::createSet(service_set,
                               areg,
                               numSchedules_);
}

void
art::ServiceDirector::
extractServices(ParameterSet const & services, ParameterSets & service_set)
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
  if (numSchedules_ == 1) { // Disabled for now.
    addService("FileCatalogMetadata", services, service_set);
  }
  if (services.get<bool>("scheduler.wantTracer", false)) {
    addService("Tracer", service_set);
  }
  ParameterSet user_services = services.get<ParameterSet>("user", ParameterSet());
  std::vector<std::string> keys = user_services.get_pset_keys();
  for (auto const & key : keys) {
    addService(key, user_services, service_set);
  }
}
