// ======================================================================
//
// ServicesManager
//
// ======================================================================

#include "art/Framework/Services/Registry/ServicesManager.h"

#include "cpp0x/utility"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <set>

using std::shared_ptr;
using fhicl::ParameterSet;
using namespace art;
using namespace std;

ServicesManager::ServicesManager(ParameterSets const& psets,
                                 LibraryManager const& lm,
                                ActivityRegistry &reg):
  registry_(reg),
  factory_(),
  index_(),
  requestedCreationOrder_(),
  actualCreationOrder_()
{
  fillFactory(psets,lm);
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
  while(!actualCreationOrder_.empty()) actualCreationOrder_.pop();
}

void ServicesManager::forceCreation()
{
  TypeIDs::iterator it(requestedCreationOrder_.begin()),
    end(requestedCreationOrder_.end());

  for(;it!=end;++it)
    {
      Factory::iterator c = factory_.find(*it);
      if(c!=factory_.end()) c->second.forceCreation(registry_);
      // JBK - should an exception be thrown if name not found in map?
    }
}

void ServicesManager::getParameterSets(ParameterSets& out) const
{
  ParameterSets tmp;
  Factory::const_iterator cur=factory_.begin(), end=factory_.end();

  for(;cur!=end;++cur)
    tmp.push_back(cur->second.getParameterSet());

  tmp.swap(out);
}

void ServicesManager::putParameterSets(ParameterSets const& n)
{
  ParameterSets::const_iterator cur=n.begin(), end=n.end();
  for(;cur!=end;++cur)
    {
      string service_name = cur->get<string>("service_type","junk");
      NameIndex::iterator ii = index_.find(service_name);
      if (ii!=index_.end()) {
        (ii->second)->second.putParameterSet(*cur);
        registry_.postServiceReconfigureSignal_(service_name);
      }
    }
}

void
  ServicesManager::fillFactory( ParameterSets  const & psets
                              , LibraryManager const & lm
                              )
{
  mf::LogVerbatim("DEBUG") << "-----> Begin ServicesManager::fillFactory()";
  typedef art::TypeID (*GET_TYPEID_t)();
  for( ParameterSets::const_iterator it = psets.begin()
                                   , e  = psets.end(); it != e; ++it )
    {
      std::string service_name(it->get<std::string>("service_type"));
      std::string service_provider(it->get<std::string>("service_provider", service_name));

      mf::LogVerbatim("DEBUG") << "-----> name:     " << service_name;
      mf::LogVerbatim("DEBUG") << "-----> provider: " << service_provider;

      // go to lm and get the typeid and maker function for this service
      GET_TYPEID_t typeid_func
         = lm.getSymbolByLibspec<GET_TYPEID_t>(service_provider,"get_typeid");
      GET_TYPEID_t interface_typeid_func
         = lm.getSymbolByLibspec<GET_TYPEID_t>(service_provider,"get_interface_typeid");
      MAKER_t make_func
         = lm.getSymbolByLibspec<MAKER_t>(service_provider,"make");
      CONVERTER_t converter_func
         = lm.getSymbolByLibspec<CONVERTER_t>(service_provider,"converter");

      if(typeid_func==0)
        throw art::Exception(art::errors::LogicError,"Service")
          << "<Could not find the get_typeid function in the service library for " << service_provider
          << "\n.  The library is probably built incorrectly.\n";

      if(make_func==0)
        throw art::Exception(art::errors::LogicError,"Service")
          << "Could not find the maker function in the service library for " << service_provider
          << "\n.  The library is probably built incorrectly.\n";

      mf::LogVerbatim("DEBUG") << "-----> Inserting provider: " << typeid_func().name();
      std::pair<Factory::iterator, bool> svc = factory_.insert(std::make_pair(
        typeid_func(), Cache(*it, typeid_func(), make_func, false, factory_.end(), converter_func)));

      if (converter_func) {
        // This service has a declared interface.
        if (service_provider == service_name) {
          std::string iface_name(cet::demangle(interface_typeid_func().name()));
          throw Exception(errors::Configuration)
	    << "Illegal use of service interface implementation as service name in configuration.\n"
            << "Correct use: services.user."
            << iface_name
            << ": { service_provider: \""
            << service_provider
            << "\" }";
        }
        mf::LogVerbatim("DEBUG") << "-----> Inserting service interface: " << interface_typeid_func().name();
	factory_.insert(std::make_pair(interface_typeid_func(),
          Cache(*it, interface_typeid_func(), make_func, true, svc.first, converter_func)));
      }

      index_[service_name]=svc.first;
      requestedCreationOrder_.push_back(typeid_func());

    }
  mf::LogVerbatim("DEBUG") << "-----> End   ServicesManager::fillFactory()";
}  // fillFactory()

namespace {
  struct NoOp {
    void operator()(ServicesManager*) {}
  };
}

// ======================================================================
