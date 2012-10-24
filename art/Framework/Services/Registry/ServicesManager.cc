// ======================================================================
//
// ServicesManager
//
// ======================================================================

#include "art/Framework/Services/Registry/ServicesManager.h"

#include "cpp0x/utility"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cassert>
#include <set>

using std::shared_ptr;
using fhicl::ParameterSet;
using namespace art;
using namespace std;

ServicesManager::ServicesManager(ParameterSets const & psets,
                                 LibraryManager const & lm,
                                 ActivityRegistry & reg):
  registry_(reg),
  factory_(),
  index_(),
  requestedCreationOrder_(),
  actualCreationOrder_()
{
  fillCache_(psets, lm);
}

ServicesManager::~ServicesManager()
{
  // Force the Service destructors to execute in the reverse order of construction.
  // Note that services passed in by a token are not included in this loop and
  // do not get destroyed until the ServicesManager object that created them is destroyed
  // which occurs after the body of this destructor is executed (the correct order).
  // Services directly passed in by a put and not created in the constructor
  // may or not be detroyed in the desired order because this class does not control
  // their creation (as I'm writing this comment everything in a standard fw
  // executable is destroyed in the desired order).
  index_.clear();
  factory_.clear();
  while (!actualCreationOrder_.empty()) { actualCreationOrder_.pop(); }
}

void ServicesManager::forceCreation()
{
  TypeIDs::iterator it(requestedCreationOrder_.begin()),
          end(requestedCreationOrder_.end());
  for (; it != end; ++it) {
    detail::ServiceCache::iterator c = factory_.find(*it);
    if (c != factory_.end()) { c->second.forceCreation(registry_); }
    // JBK - should an exception be thrown if name not found in map?
  }
}

void ServicesManager::getParameterSets(ParameterSets & out) const
{
  ParameterSets tmp;
  detail::ServiceCache::const_iterator cur = factory_.begin(), end = factory_.end();
  for (; cur != end; ++cur)
  { tmp.push_back(cur->second.getParameterSet()); }
  tmp.swap(out);
}

void ServicesManager::putParameterSets(ParameterSets const & n)
{
  ParameterSets::const_iterator cur = n.begin(), end = n.end();
  for (; cur != end; ++cur) {
    string service_name = cur->get<string>("service_type", "junk");
    NameIndex::iterator ii = index_.find(service_name);
    if (ii != index_.end()) {
      (ii->second)->second.putParameterSet(*cur);
      registry_.sPostServiceReconfigure_(service_name);
    }
  }
}

void
ServicesManager::fillCache_(ParameterSets  const & psets,
                              LibraryManager const & lm)
{
 // Receive from Eve ntProcessor when we go multi-schedule.
  detail::ServiceCacheEntry::setNSchedules(1);

  // Loop over each configured service parameter set.
  for (auto const & ps : psets) {
    std::string service_name(ps.get<std::string>("service_type"));
    std::string service_provider(ps.get<std::string>("service_provider", service_name));
    // Get the helper from the library.
    std::unique_ptr<detail::ServiceHelperBase> service_helper
    { lm.getSymbolByLibspec<SHBCREATOR_t>(service_provider,
                                          "create_service_helper")() };
    assert(!service_helper->is_interface() &&
           "Registered service is not a service implementation!");
    std::unique_ptr<detail::ServiceInterfaceHelper> iface_helper;
    if (service_helper->is_interface_impl()) { // Expect an interface helper
      iface_helper.reset(dynamic_cast<detail::ServiceInterfaceHelper*>
                         (lm.getSymbolByLibspec<SHBCREATOR_t>
                          (service_provider,
                           "create_iface_helper")().release()));
      assert(dynamic_cast<detail::ServiceInterfaceImplHelper *>(service_helper.get())->get_interface_typeid() ==
             iface_helper->get_typeid() &&
             "Found unexepcted interface!");
      if (service_provider == service_name) {
        std::string iface_name(cet::demangle(iface_helper->get_typeid().name()));
        throw Exception(errors::Configuration)
          << "Illegal use of service interface implementation as service name in configuration.\n"
          << "Correct use: services.user."
          << iface_name
          << ": { service_provider: \""
          << service_provider
          << "\" }";
      }
    }
    // Insert the cache entry for the main service implementation. Note
    // we save the typeid of the implementation because we're about to
    // give away the helper.
    TypeID service_typeid(service_helper->get_typeid());
    auto svc = insertImpl_(ps, std::move(service_helper));
    if (iface_helper) {
      insertInterface_(ps, std::move(iface_helper), svc.first);
    }
    index_[service_name] = svc.first;
    requestedCreationOrder_.emplace_back(std::move(service_typeid));
  }
}  // fillCache()

std::pair<art::detail::ServiceCache::iterator, bool>
art::ServicesManager::
insertImpl_(fhicl::ParameterSet const & pset,
            std::unique_ptr<detail::ServiceHelperBase> && helper)
{
  // Need temporary because we can't guarantee the order of evaluation
  // of the arguments to std::make_pair() below.
  TypeID sType(helper->get_typeid());
  return
    factory_.insert(std::make_pair(sType,
                                   detail::ServiceCacheEntry(pset,
                                         std::move(helper))));
}

void
art::ServicesManager::
  insertInterface_(fhicl::ParameterSet const & pset,
                   std::unique_ptr<detail::ServiceHelperBase> && helper,
                   detail::ServiceCache::iterator implEntry)
{
  // Need temporary because we can't guarantee the order of evaluation
  // of the arguments to std::make_pair() below.
  TypeID iType(helper->get_typeid());
  factory_.insert(std::make_pair(iType,
                                 detail::ServiceCacheEntry(pset,
                                       std::move(helper),
                                       implEntry)));
}
// ======================================================================
