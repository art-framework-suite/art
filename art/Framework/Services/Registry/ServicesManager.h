#ifndef art_Framework_Services_Registry_ServicesManager_h
#define art_Framework_Services_Registry_ServicesManager_h

////////////////////////////////////////////////////////////////////////
// ServicesManager
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceCache.h"
#include "art/Framework/Services/Registry/detail/ServiceCacheEntry.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Framework/Services/Registry/detail/ServiceStack.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapper.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/demangle.h"

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace cet {
  class LibraryManager;
}

namespace art {
  class ServicesManager;

  class ActivityRegistry;
}

namespace fhicl {
  class ParameterSet;
}

class art::ServicesManager {
public:
  // non-copyable:
  ServicesManager(ServicesManager const &) = delete;
  ServicesManager& operator= (ServicesManager const &) = delete;

  using ParameterSets = std::vector<fhicl::ParameterSet>;

public:
  ServicesManager(ParameterSets const &,
                  cet::LibraryManager const &,
                  ActivityRegistry &);

  ~ServicesManager();

  template < class T,
           typename = std::enable_if_t < detail::ServiceHelper<T>::scope_val != ServiceScope::PER_SCHEDULE > >
  T &
  get();

  template < class T,
           typename = std::enable_if_t<detail::ServiceHelper<T>::scope_val == ServiceScope::PER_SCHEDULE> >
  T &
  get(ScheduleID);

  // Returns true of the particular service is accessible -- that is, it
  // either can be made (if requested) or has already been made.
  template< class T >
  bool
  isAvailable() const {
    return factory_.find(TypeID(typeid(T))) != factory_.end();
  }

  template < typename T,
           typename = std::enable_if_t < detail::ServiceHelper<T>::scope_val != ServiceScope::PER_SCHEDULE > >
  void
  put(std::unique_ptr<T> && premade_service);

  // SP<T> must be convertible to std::shared_ptr<T>.
  template < class SP,
           typename = std::enable_if_t < detail::ServiceHelper<typename SP::element_type>::scope_val ==
           ServiceScope::PER_SCHEDULE > >
  void
  put(std::vector<SP> && premade_services);  // force all the services that are not alrady made into existance

  // using 'reg'.  The order of creation will be the registration order.
  void forceCreation();

  void getParameterSets(ParameterSets & out) const;
  void putParameterSets(ParameterSets const &);

private:
  typedef std::unique_ptr<detail::ServiceHelperBase> (*SHBCREATOR_t)();

  typedef  std::map< std::string, detail::ServiceCache::iterator > NameIndex;
  typedef  std::vector< TypeID >      TypeIDs;

  void fillCache_(ParameterSets const & psets, cet::LibraryManager const & lm);

  std::pair<detail::ServiceCache::iterator, bool>
  insertImpl_(fhicl::ParameterSet const & pset,
              std::unique_ptr<detail::ServiceHelperBase> && helper);

  void
  insertInterface_(fhicl::ParameterSet const & pset,
                   std::unique_ptr<detail::ServiceHelperBase> && helper,
                   detail::ServiceCache::iterator implEntry);

  // these are real things that we use.
  art::ActivityRegistry & registry_;
  detail::ServiceCache factory_;
  NameIndex index_;

  TypeIDs requestedCreationOrder_;
  detail::ServiceStack actualCreationOrder_;
  std::vector<std::string> configErrMsgs_;

};  // ServicesManager

// ----------------------------------------------------------------------

template < class T,
         typename >
T &
art::ServicesManager::get() {
  // Find the correct ServiceCacheEntry object.
  detail::ServiceCache::iterator it = factory_.find(TypeID(typeid(T)));
  if (it == factory_.end())
    throw art::Exception(art::errors::ServiceNotFound, "Service")
        << " unable to find requested service with compiler type name '"
        << cet::demangle_symbol(typeid(T).name()) << "'.\n";
  return it->second.get<T>(registry_, actualCreationOrder_);
}  // get<>()

template < class T,
         typename >
T &
art::ServicesManager::get(ScheduleID sID) {
  // Find the correct ServiceCacheEntry object.
  detail::ServiceCache::iterator it = factory_.find(TypeID(typeid(T)));
  if (it == factory_.end())
    throw art::Exception(art::errors::ServiceNotFound, "Service")
        << " unable to find requested service with compiler type name '"
        << cet::demangle_symbol(typeid(T).name()) << "'.\n";
  return it->second.get<T>(registry_, actualCreationOrder_, sID);
}  // get<>()

template <typename T, typename>
void
art::ServicesManager::put(std::unique_ptr<T> && premade_service)
{
  std::unique_ptr<detail::ServiceHelperBase>
  service_helper(new detail::ServiceHelper<T>);
  TypeID id(typeid(T));
  detail::ServiceCache::const_iterator it = factory_.find(id);
  if (it != factory_.end()) {
    throw art::Exception(art::errors::LogicError, "Service")
        << "The system has manually added service of type "
        << cet::demangle_symbol(id.name())
        << ", but the service system already has a configured service"
        << " of that type\n";
  }
  detail::WrapperBase_ptr swb(new detail::ServiceWrapper<T, detail::ServiceHelper<T>::scope_val>(std::move(premade_service)));
  actualCreationOrder_.push(swb);
  factory_.insert(std::make_pair(id, detail::ServiceCacheEntry(std::move(swb),
                                 std::move(service_helper))));
}

template <class SP, typename>
void
art::ServicesManager::put(std::vector<SP> && premade_services)
{
  typedef typename SP::element_type element_type;
  std::unique_ptr<detail::ServiceHelperBase>
  service_helper(new detail::ServiceHelper<element_type>);
  TypeID id(typeid(element_type));
  detail::ServiceCache::const_iterator it = factory_.find(id);
  if (it != factory_.end()) {
    throw art::Exception(art::errors::LogicError, "Service:")
        << "The system has manually added service of type "
        << cet::demangle_symbol(id.name())
        << ", but the service system already has a configured service"
        << " of that type\n";
  }
  detail::WrapperBase_ptr swb(new detail::ServiceWrapper<element_type, detail::ServiceHelper<element_type>::scope_val>(std::move(premade_services)));
  actualCreationOrder_.push(swb);
  factory_.insert(std::make_pair(id, detail::ServiceCacheEntry(std::move(swb),
                                 std::move(service_helper))));
}

// ======================================================================

#endif /* art_Framework_Services_Registry_ServicesManager_h */

// Local Variables:
// mode: c++
// End:
