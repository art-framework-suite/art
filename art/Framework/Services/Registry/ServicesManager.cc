// ======================================================================
//
// ServicesManager
//
// ======================================================================

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "cetlib/LibraryManager.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/detail/validationException.h"

#include <cassert>
#include <memory>
#include <utility>

art::ServicesManager::
ServicesManager(ParameterSets const& psets,
                cet::LibraryManager const& lm,
                ActivityRegistry& reg):
  registry_{reg}
{
  fillCache_(psets, lm);
}

art::ServicesManager::~ServicesManager()
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
  index_.clear();
  factory_.clear();
  while (!actualCreationOrder_.empty()) { actualCreationOrder_.pop(); }
}

void
art::ServicesManager::forceCreation()
{
  for (auto const& val : requestedCreationOrder_) {
    auto c = factory_.find(val);
    if (c != factory_.end()) { c->second.forceCreation(registry_); }
    // JBK - should an exception be thrown if name not found in map?
  }
}

void
art::ServicesManager::getParameterSets(ParameterSets& out) const
{
  ParameterSets tmp;
  for (auto const& cur : factory_) {
    tmp.push_back(cur.second.getParameterSet());
  }
  tmp.swap(out);
}

void
art::ServicesManager::fillCache_(ParameterSets const& psets, cet::LibraryManager const& lm)
{
  // Receive from EventProcessor when we go multi-schedule.
  detail::ServiceCacheEntry::setNSchedules(1);
  // Loop over each configured service parameter set.
  for (auto const& ps : psets) {
    std::string const service_name {ps.get<std::string>("service_type")};
    std::string const service_provider {ps.get<std::string>("service_provider", service_name)};
    // Get the helper from the library.
    std::unique_ptr<detail::ServiceHelperBase> service_helper {
      lm.getSymbolByLibspec<SHBCREATOR_t>(service_provider,"create_service_helper")()
    };
    if (service_helper->is_interface()) {
      throw Exception(errors::LogicError)
        << "Service "
        << service_name
        << " (of type "
        << service_helper->get_typeid().className()
        << ")\nhas been registered as an interface in its header using\n"
        << "DECLARE_ART_SERVICE_INTERFACE.\n"
        << "Use DECLARE_ART_SERVICE OR DECLARE_ART_SERVICE_INTERFACE_IMPL\n"
        << "as appropriate. A true service interface should *not* be\n"
        << "compiled into a  _service.so plugin library.\n";
    }
    std::unique_ptr<detail::ServiceInterfaceHelper> iface_helper;
    if (service_helper->is_interface_impl()) { // Expect an interface helper
      iface_helper.reset(dynamic_cast<detail::ServiceInterfaceHelper*>
                         (lm.getSymbolByLibspec<SHBCREATOR_t>
                          (service_provider,
                           "create_iface_helper")().release()));
      if (dynamic_cast<detail::ServiceInterfaceImplHelper*>(service_helper.get())->get_interface_typeid() !=
          iface_helper->get_typeid()) {
        throw Exception(errors::LogicError)
          << "Service registration for "
          << service_provider
          << " is internally inconsistent: "
          << iface_helper->get_typeid()
          << " ("
          << iface_helper->get_typeid().className()
          << ") != "
          << dynamic_cast<detail::ServiceInterfaceImplHelper*>(service_helper.get())->get_interface_typeid()
          << " ("
          << dynamic_cast<detail::ServiceInterfaceImplHelper*>(service_helper.get())->get_interface_typeid().className()
          << ").\n"
          << "Contact the art developers <artists@fnal.gov>.\n";
      }
      if (service_provider == service_name) {
        std::string const iface_name {cet::demangle_symbol(iface_helper->get_typeid().name())};
        throw Exception(errors::Configuration)
            << "Illegal use of service interface implementation as service name in configuration.\n"
            << "Correct use: services."
            << iface_name
            << ": { service_provider: \""
            << service_provider
            << "\" }\n";
      }
    }
    // Insert the cache entry for the main service implementation. Note
    // we save the typeid of the implementation because we're about to
    // give away the helper.
    TypeID service_typeid{service_helper->get_typeid()};
    auto svc = insertImpl_(ps, std::move(service_helper));
    if (iface_helper) {
      insertInterface_(ps, std::move(iface_helper), svc.first);
    }
    index_[service_name] = svc.first;
    requestedCreationOrder_.emplace_back(std::move(service_typeid));
  }
}  // fillCache()

std::pair<art::detail::ServiceCache::iterator, bool>
art::ServicesManager::insertImpl_(fhicl::ParameterSet const& pset,
                                  std::unique_ptr<detail::ServiceHelperBase>&& helper)
{
  // Need temporary because we can't guarantee the order of evaluation
  // of the arguments to emplace below.
  TypeID const sType {helper->get_typeid()};
  return factory_.emplace(sType,
                          detail::ServiceCacheEntry(pset,
                                                    std::move(helper)));
}

void
art::ServicesManager::insertInterface_(fhicl::ParameterSet const& pset,
                                       std::unique_ptr<detail::ServiceHelperBase>&& helper,
                                       detail::ServiceCache::iterator const implEntry)
{
  // Need temporary because we can't guarantee the order of evaluation
  // of the arguments to emplace below.
  TypeID const iType {helper->get_typeid()};
  factory_.emplace(iType,
                   detail::ServiceCacheEntry(pset,
                                             std::move(helper),
                                             implEntry->second));

}
// ======================================================================
