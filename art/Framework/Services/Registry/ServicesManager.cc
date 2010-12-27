// ======================================================================
//
// ServicesManager
//
// ======================================================================

#include "art/Framework/Services/Registry/ServicesManager.h"

#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "fhiclcpp/ParameterSet.h"
#include <set>
#include <utility>  // make_pair

using boost::shared_ptr;
using fhicl::ParameterSet;
using namespace art;
using namespace std;

ServicesManager::ServicesManager(ParameterSets const& psets, LibraryManager const& lm):
  associatedManager_(),
  registry_(),
  factory_(),
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


void
  ServicesManager::fillFactory( ParameterSets  const & psets
                              , LibraryManager const & lm
                              )
{
  for( ParameterSets::const_iterator it = psets.begin()
                                   , e  = psets.end(); it != e; ++it )
    {
      string service_name = it->get<string>("service_type");

      // go to lm and get the typeid and maker function for this service
      GET_TYPEID_t typeid_func
        = (GET_TYPEID_t)lm.getSymbolByLibspec(service_name,"get_typeid");
      MAKER_t make_func
        = (MAKER_t)lm.getSymbolByLibspec(service_name,"make");

      if(typeid_func==0)
        throw art::Exception(art::errors::LogicError,"Service")
          << "<Could not find the get_typeid function in the service library for " << service_name
          << "\n.  The library is probably built incorrectly.\n";

      if(make_func==0)
        throw art::Exception(art::errors::LogicError,"Service")
          << "Could not find the maker function in the service library for " << service_name
          << "\n.  The library is probably built incorrectly.\n";

      TypeIDBase id = typeid_func();

      // insert cache object for it
      factory_.insert( std::make_pair(id, Cache(*it, id, make_func)) );
      requestedCreationOrder_.push_back(id);
    }
}  // fillFactory()

namespace {
  struct NoOp {
    void operator()(ServicesManager*) {}
  };
}

// ======================================================================
