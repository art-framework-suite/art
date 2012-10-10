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
  fillFactory_(psets, lm);
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
    Factory::iterator c = factory_.find(*it);
    if (c != factory_.end()) { c->second.forceCreation(registry_); }
    // JBK - should an exception be thrown if name not found in map?
  }
}

void ServicesManager::getParameterSets(ParameterSets & out) const
{
  ParameterSets tmp;
  Factory::const_iterator cur = factory_.begin(), end = factory_.end();
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
ServicesManager::fillFactory_(ParameterSets  const & psets,
                              LibraryManager const & lm)
{
  typedef art::TypeID(*GET_TYPEID_t)();
  for (ParameterSets::const_iterator it = psets.begin()
                                          , e  = psets.end(); it != e; ++it) {
    std::string service_name(it->get<std::string>("service_type"));
    std::string service_provider(it->get<std::string>("service_provider", service_name));
    // go to lm and get the typeid and maker function for this service
    GET_TYPEID_t typeid_func
      = lm.getSymbolByLibspec<GET_TYPEID_t>(service_provider, "get_typeid");
    MAKER_t make_func
      = lm.getSymbolByLibspec<MAKER_t>(service_provider, "make");
    GET_TYPEID_t iface_impl_typeid_func
      = lm.getSymbolByLibspec<GET_TYPEID_t>(service_provider, "get_iface_impl_typeid", LibraryManager::nothrow);
    CONVERTER_t converter_func(nullptr);
    GET_TYPEID_t iface_typeid_func(nullptr);
    // Check if his service has a declared interface.
    if (iface_impl_typeid_func &&
        iface_impl_typeid_func() == typeid_func()) {
      // Note that we check for equality because it's possible that we
      // could pick up the interface functions from a library to which
      // our current library is linked, rather than our current library itself.
      converter_func
        = lm.getSymbolByLibspec<CONVERTER_t>(service_provider, "converter");
      iface_typeid_func
        = lm.getSymbolByLibspec<GET_TYPEID_t>(service_provider, "get_iface_typeid");
      if (service_provider == service_name) {
        std::string iface_name(cet::demangle(iface_typeid_func().name()));
        throw Exception(errors::Configuration)
            << "Illegal use of service interface implementation as service name in configuration.\n"
            << "Correct use: services.user."
            << iface_name
            << ": { service_provider: \""
            << service_provider
            << "\" }";
      }
    }
    // Insert the cache object for the main service implementation,
    // including the converter to interface (converter_func) if defined.
    auto svc = insertImpl_(typeid_func(), *it, make_func, converter_func);
    if (converter_func) {
      // Insert the cache object for the interface.
      assert(iface_typeid_func);
      insertInterface_(iface_typeid_func(), *it, svc.first);
    }
    index_[service_name] = svc.first;
    requestedCreationOrder_.push_back(typeid_func());
  }
}  // fillFactory()

std::pair<art::ServicesManager::Factory::iterator, bool>
art::ServicesManager::
insertImpl_(TypeID implType,
            fhicl::ParameterSet const &pset,
            MAKER_t make,
            CONVERTER_t convert)
{
  return
    factory_.insert(std::make_pair(implType,
                                   Cache(pset,
                                         implType,
                                         make,
                                         convert)));
}

void
art::ServicesManager::
insertInterface_(TypeID ifaceType,
                 fhicl::ParameterSet const &pset,
                 Factory::iterator implEntry)
{
  factory_.insert(std::make_pair(ifaceType,
                                 Cache(pset,
                                       ifaceType,
                                       implEntry)));
}

namespace {
  struct NoOp {
    void operator()(ServicesManager *) {}
  };
}

// ======================================================================
