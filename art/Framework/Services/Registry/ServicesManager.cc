#include "art/Framework/Services/Registry/ServicesManager.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceCacheEntry.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Utilities/bold_fontify.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/LibraryManager.h"
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
    // Force the Service destructors to execute in the reverse order of
    // construction.  Note that services passed in by a token are not
    // included in this loop and do not get destroyed until the
    // ServicesManager object that created them is destroyed which
    // occurs after the body of this destructor is executed (the correct
    // order).  Services directly passed in by a put and not created in
    // the constructor may or may not be detroyed in the desired order
    // because this class does not control their creation (as I'm
    // writing this comment everything in a standard fw executable is
    // destroyed in the desired order).
    factory_.clear();
    while (!actualCreationOrder_.empty()) {
      actualCreationOrder_.pop();
    }
  }

  void
  ServicesManager::registerProducts(ProductDescriptions& productsToProduce,
                                    ProducingServiceSignals& signals,
                                    ProcessConfiguration const& pc)
  {
    for (auto& pr : factory_) {
      auto& serviceEntry = pr.second;

      // Per-schedule services cannot register products
      if (serviceEntry.serviceScope() == ServiceScope::PER_SCHEDULE)
        continue;

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

      // The enum value SHARED is not used below so as to avoid circular
      // dependencies.
      ModuleDescription const md{
        pset.id(), moduleLabel, moduleLabel, 2 /*==SHARED*/, pc};
      serviceEntry.registerProducts(productsToProduce, signals, md);
    }
  }

  ServicesManager::ServicesManager(ParameterSet&& servicesPSet,
                                   ActivityRegistry& actReg)
    : actReg_{actReg}
    , lm_{Suffixes::service()}
    , factory_{}
    , requestedCreationOrder_{}
    , actualCreationOrder_{}
    , configErrMsgs_()
  {
    vector<ParameterSet> psets;
    {
      servicesPSet.erase("scheduler");
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
      string const service_name{ps.get<string>("service_type")};
      string const service_provider{
        ps.get<string>("service_provider", service_name)};
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
      auto svc = factory_.emplace(
        sType, detail::ServiceCacheEntry(ps, move(service_helper)));

      if (iface_helper) {
        // Need temporary because we can't guarantee the order of evaluation
        // of the arguments to make_pair() below.
        TypeID const iType{iface_helper->get_typeid()};
        factory_.emplace(
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
    for (auto const& typeID_and_ServiceCacheEntry : factory_) {
      auto const& sce = typeID_and_ServiceCacheEntry.second;
      tmp.push_back(sce.getParameterSet());
    }
    tmp.swap(out);
  }

  void
  ServicesManager::forceCreation()
  {
    for (auto const& typeID : requestedCreationOrder_) {
      auto I = factory_.find(typeID);
      if (I != factory_.end()) {
        auto const& sce = I->second;
        sce.forceCreation(actReg_);
      }
    }
  }

} // namespace art
