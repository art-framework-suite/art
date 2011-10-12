// ======================================================================
//
// ServicesManager
//
// ======================================================================

#include "art/Framework/Services/Registry/ServicesManager.h"

#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "cpp0x/utility"
#include "fhiclcpp/ParameterSet.h"
#include <set>

using std::shared_ptr;
using fhicl::ParameterSet;
using namespace art;
using namespace std;

ServicesManager::ServicesManager(ParameterSets const& psets, LibraryManager const& lm):
  associatedManager_(),
  registry_(),
  factory_(),
  index_(),
  requestedCreationOrder_(),
  actualCreationOrder_()
{
  fillFactory(psets,lm);
}

// this inheritance procedure needs to go away...
ServicesManager::ServicesManager(ServiceToken iToken,
                                 ServiceLegacy iLegacy,
                                 ParameterSets const& psets,
                                 LibraryManager const& lm):
  associatedManager_(iToken.manager_)
{
  throw "You are calling the inheritance ctor of ServiceManager and should not be doing that!";
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

void
ServicesManager::connect(ActivityRegistry& iOther)
{
  registry_.connect(iOther);
}

void
ServicesManager::connectTo(ActivityRegistry& iOther)
{
  iOther.connect(registry_);
}

void
ServicesManager::copySlotsFrom(ActivityRegistry& iOther)
{
  registry_.copySlotsFrom(iOther);
}

void
ServicesManager::copySlotsTo(ActivityRegistry& iOther)
{
  iOther.copySlotsFrom(registry_);
}

void ServicesManager::forceCreation(ActivityRegistry& reg)
{
  TypeIDs::iterator it(requestedCreationOrder_.begin()),
    end(requestedCreationOrder_.end());

  for(;it!=end;++it)
    {
      Factory::iterator c = factory_.find(*it);
      if(c!=factory_.end()) c->second.forceCreation(reg);
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
      if(ii!=index_.end())
        (ii->second)->second.putParameterSet(*cur);
    }
}

void
  ServicesManager::fillFactory( ParameterSets  const & psets
                              , LibraryManager const & lm
                              )
{
  typedef art::TypeID (*GET_TYPEID_t)();
  for( ParameterSets::const_iterator it = psets.begin()
                                   , e  = psets.end(); it != e; ++it )
    {
      string service_name = it->get<string>("service_type","junk");

      // go to lm and get the typeid and maker function for this service
      GET_TYPEID_t typeid_func
         = reinterpret_cast<GET_TYPEID_t>(lm.getSymbolByLibspec(service_name,"get_typeid"));
      MAKER_t make_func
         = reinterpret_cast<MAKER_t>(lm.getSymbolByLibspec(service_name,"make"));

      if(typeid_func==0)
        throw art::Exception(art::errors::LogicError,"Service")
          << "<Could not find the get_typeid function in the service library for " << service_name
          << "\n.  The library is probably built incorrectly.\n";

      if(make_func==0)
        throw art::Exception(art::errors::LogicError,"Service")
          << "Could not find the maker function in the service library for " << service_name
          << "\n.  The library is probably built incorrectly.\n";

      TypeID id = typeid_func();

      // insert cache object for it
      std::pair<Factory::iterator,bool> ib=factory_.insert( std::make_pair(id, Cache(*it, id, make_func)) );

      if(service_name!="junk")
        index_[service_name]=ib.first;
      requestedCreationOrder_.push_back(id);
    }
}  // fillFactory()

namespace {
  struct NoOp {
    void operator()(ServicesManager*) {}
  };
}

// ======================================================================
