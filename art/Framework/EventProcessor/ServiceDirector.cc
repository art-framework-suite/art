#include "art/Framework/EventProcessor/ServiceDirector.h"

#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "fhiclcpp/ParameterSetRegistry.h"

using fhicl::ParameterSet;
using ParameterSets = std::vector<ParameterSet>;

namespace {

  void
  addService(std::string const& name, ParameterSets& service_set)
  {
    ParameterSet tmp;
    tmp.put("service_type", name);
    fhicl::ParameterSetRegistry::put(tmp);
    service_set.emplace_back(std::move(tmp));
  }

  void
  addService(std::string const& name,
             ParameterSet const& source,
             ParameterSets& service_set)
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
  extractServices(ParameterSet&& services)
  {
    ParameterSets service_set;
    bool const wantTracer {services.get<bool>("scheduler.wantTracer", false)};
    services.erase("scheduler");

    // If we want the tracer and it's not explicitly configured, insert
    // it, otherwise it'll get picked up automatically.
    if (wantTracer && !services.has_key("Tracer")) {
      addService("Tracer", service_set);
    }

    // Force presence of FileCatalogMetadata service.
    addService("FileCatalogMetadata", services, service_set);
    services.erase("FileCatalogMetadata");

    // Force presence of DatabaseConnection service.
    addService("DatabaseConnection", services, service_set);
    services.erase("DatabaseConnection");

    // Extract all
    for (auto const& key : services.get_pset_names()) {
      addService(key, services, service_set);
    }
    return std::move(service_set);
  }
}

art::ServiceDirector::
ServiceDirector(fhicl::ParameterSet&& services,
                ActivityRegistry& areg,
                ServiceToken& serviceToken):
  serviceToken_{serviceToken}
{
  serviceToken_ = ServiceRegistry::createSet(extractServices(std::move(services)), areg);
}
