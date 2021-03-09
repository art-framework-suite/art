#include "art/Framework/Services/Registry/ServicesManager.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceCacheEntry.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/PluginSuffixes.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/LibraryManager.h"
#include "cetlib/bold_fontify.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/types/detail/validationException.h"

#include <cassert>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using fhicl::ParameterSet;
using fhicl::ParameterSetRegistry;

namespace {

  void
  addService(string const& name, vector<ParameterSet>& service_set)
  {
    ParameterSet tmp;
    tmp.put("service_type", name);
    ParameterSetRegistry::put(tmp);
    service_set.emplace_back(move(tmp));
  }

  void
  addService(string const& name,
             ParameterSet const& source,
             vector<ParameterSet>& service_set)
  {
    ParameterSet tmp;
    if (source.get_if_present(name, tmp)) {
      service_set.emplace_back(move(tmp));
      return;
    }
    addService(name, service_set);
  }

} // unnamed namespace

namespace art {

  ServicesManager::~ServicesManager()
  {
    // Force the Service destructors to execute in the reverse order
    // of construction.  We first clear the the services cache, which
    // is safe to do as the services owned by it are co-owned by the
    // actualCreationOrder_ data member.
    services_.clear();
    while (!actualCreationOrder_.empty()) {
      actualCreationOrder_.pop();
    }
  }

  std::vector<std::string>
  ServicesManager::registerProducts(ProductDescriptions& productsToProduce,
                                    ProducingServiceSignals& signals,
                                    ProcessConfiguration const& pc)
  {
    std::vector<std::string> producing_services;
    for (auto& pr : services_) {
      auto& serviceEntry = pr.second;

      // Service interfaces cannot be used for product insertion.
      if (serviceEntry.is_interface())
        continue;

      // The value of service_type becomes the "module name/label" for
      // the ModuleDescription object.
      auto const& pset = serviceEntry.getParameterSet();
      std::string moduleLabel{};
      if (!pset.get_if_present("service_type", moduleLabel)) {
        // System services do not insert products.
        continue;
      }

      auto const before = productsToProduce.size();
      ModuleDescription const md{
        pset.id(), moduleLabel, moduleLabel, ModuleThreadingType::shared, pc};
      serviceEntry.registerProducts(productsToProduce, signals, md);
      if (productsToProduce.size() != before) {
        // Registering the products for this service has changed the
        // size of productsToProduce.  We make the reasonable
        // assumption that productsToProduce has increased in size.
        producing_services.push_back(moduleLabel);
      }
    }
    return producing_services;
  }

  ServicesManager::ServicesManager(ParameterSet&& servicesPSet,
                                   ActivityRegistry& actReg,
                                   detail::SharedResources& resources)
    : actReg_{actReg}, resources_{resources}
  {
    vector<ParameterSet> psets;
    {
      // Force presence of FileCatalogMetadata service.
      addService("FileCatalogMetadata", servicesPSet, psets);
      servicesPSet.erase("FileCatalogMetadata");
      // Force presence of DatabaseConnection service.
      addService("DatabaseConnection", servicesPSet, psets);
      servicesPSet.erase("DatabaseConnection");
      // Extract all
      for (auto const& key : servicesPSet.get_pset_names()) {
        addService(key, servicesPSet, psets);
      }
    }
    using SHBCREATOR_t = std::unique_ptr<detail::ServiceHelperBase> (*)();
    for (auto const& ps : psets) {
      auto const service_name = ps.get<string>("service_type");
      auto const service_provider =
        ps.get<string>("service_provider", service_name);
      // Get the helper from the library.
      unique_ptr<detail::ServiceHelperBase> service_helper{
        lm_.getSymbolByLibspec<SHBCREATOR_t>(service_provider,
                                             "create_service_helper")()};
      if (service_helper->is_interface()) {
        throw Exception(errors::LogicError)
          << "Service " << service_name << " (of type "
          << service_helper->get_typeid().className()
          << ")\nhas been registered as an interface in its header using\n"
          << "DECLARE_ART_SERVICE_INTERFACE.\n"
          << "Use DECLARE_ART_SERVICE OR DECLARE_ART_SERVICE_INTERFACE_IMPL\n"
          << "as appropriate. A true service interface should *not* be\n"
          << "compiled into a  _service.so plugin library.\n";
      }
      unique_ptr<detail::ServiceInterfaceHelper> iface_helper;
      if (service_helper->is_interface_impl()) { // Expect an interface helper
        iface_helper.reset(dynamic_cast<detail::ServiceInterfaceHelper*>(
          lm_
            .getSymbolByLibspec<SHBCREATOR_t>(service_provider,
                                              "create_iface_helper")()
            .release()));
        if (dynamic_cast<detail::ServiceInterfaceImplHelper*>(
              service_helper.get())
              ->get_interface_typeid() != iface_helper->get_typeid()) {
          throw Exception(errors::LogicError)
            << "Service registration for " << service_provider
            << " is internally inconsistent: " << iface_helper->get_typeid()
            << " (" << iface_helper->get_typeid().className() << ") != "
            << dynamic_cast<detail::ServiceInterfaceImplHelper*>(
                 service_helper.get())
                 ->get_interface_typeid()
            << " ("
            << dynamic_cast<detail::ServiceInterfaceImplHelper*>(
                 service_helper.get())
                 ->get_interface_typeid()
                 .className()
            << ").\n"
            << "Contact the art developers <artists@fnal.gov>.\n";
        }
        if (service_provider == service_name) {
          string iface_name{
            cet::demangle_symbol(iface_helper->get_typeid().name())};
          // Remove any namespace qualification if necessary
          auto const colon_pos = iface_name.find_last_of(":");
          if (colon_pos != std::string::npos) {
            iface_name.erase(0, colon_pos + 1);
          }
          throw Exception(errors::Configuration)
            << "Illegal use of service interface implementation as service "
               "name in configuration.\n"
            << "Correct use: services." << iface_name
            << ": { service_provider: \"" << service_provider << "\" }\n";
        }
      }
      // Insert the cache entry for the main service implementation. Note
      // we save the typeid of the implementation because we're about to
      // give away the helper.
      TypeID service_typeid{service_helper->get_typeid()};

      // Need temporary because we can't guarantee the order of evaluation
      // of the arguments to make_pair() below.
      TypeID const sType{service_helper->get_typeid()};
      auto svc = services_.emplace(
        sType, detail::ServiceCacheEntry(ps, move(service_helper)));

      if (iface_helper) {
        // Need temporary because we can't guarantee the order of evaluation
        // of the arguments to make_pair() below.
        TypeID const iType{iface_helper->get_typeid()};
        services_.emplace(
          iType,
          detail::ServiceCacheEntry(ps, move(iface_helper), svc.first->second));
      }
      requestedCreationOrder_.emplace_back(move(service_typeid));
    }
  }

  void
  ServicesManager::getParameterSets(std::vector<fhicl::ParameterSet>& out) const
  {
    std::vector<fhicl::ParameterSet> tmp;
    for (auto const& typeID_and_ServiceCacheEntry : services_) {
      auto const& sce = typeID_and_ServiceCacheEntry.second;
      tmp.push_back(sce.getParameterSet());
    }
    tmp.swap(out);
  }

  void
  ServicesManager::forceCreation()
  {
    for (auto const& typeID : requestedCreationOrder_) {
      if (auto it = services_.find(typeID); it != services_.end()) {
        auto const& sce = it->second;
        sce.forceCreation(actReg_, resources_);
      }
    }
  }

} // namespace art
