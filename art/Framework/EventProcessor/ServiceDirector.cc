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

  ParameterSets
  extractServices(ParameterSet & services)
  {
    ParameterSets service_set;
    bool const wantTracer = services.get<bool>("scheduler.wantTracer", false);
    services.erase("scheduler");

    // If we want the tracer and it's not explicitly configured, insert
    // it, otherwise it'll get picked up automatically.
    if (wantTracer && ! services.has_key("Tracer")) {
      addService("Tracer", service_set);
    }

    // Force presence of FileCatalogMetadata service.
    addService("FileCatalogMetadata", services, service_set);
    services.erase("FileCatalogMetadata");

    // Extract all
    for (auto const & key : services.get_pset_keys()) {
      addService(key, services, service_set);
    }
    return std::move(service_set);
  }
}

art::ServiceDirector::
ServiceDirector(fhicl::ParameterSet services,
                ActivityRegistry & areg,
                ServiceToken & serviceToken)
:
  serviceToken_(serviceToken)
{
  serviceToken_ = ServiceRegistry::createSet(extractServices(services), areg);
}
