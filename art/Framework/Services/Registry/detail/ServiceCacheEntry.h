#ifndef art_Framework_Services_Registry_detail_ServiceCacheEntry_h
#define art_Framework_Services_Registry_detail_ServiceCacheEntry_h
////////////////////////////////////////////////////////////////////////
// ServiceCacheEntry
//
// Used by ServicesManager to handle creation and caching of services.

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceCache.h"
#include "art/Framework/Services/Registry/detail/ServiceStack.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

#include <cassert>

namespace art {
  namespace detail {
    class ServiceCacheEntry;
  }

  // Forward declarations.
  class ActivityRegistry;
}

// A ServiceCacheEntry object encapsulates the (possibly delayed)
// creation of a Service object.
class art::detail::ServiceCacheEntry {
public:
  // Service implementation.
  ServiceCacheEntry(fhicl::ParameterSet const & pset,
                    std::unique_ptr<detail::ServiceHelperBase> && helper);

  // Service interface implementation.
  ServiceCacheEntry(fhicl::ParameterSet const & pset,
                    std::unique_ptr<detail::ServiceHelperBase> && helper,
                    ServiceCache::iterator impl);

  // Pre-made service (1).
  ServiceCacheEntry(WrapperBase_ptr premade_service,
                    std::unique_ptr<detail::ServiceHelperBase> && helper);

  // Create the service if necessary, and return the WrapperBase_ptr
  // that refers to it.
  WrapperBase_ptr getService(ActivityRegistry & reg,
                             detail::ServiceStack & creationOrder) const;

  void forceCreation(ActivityRegistry & reg) const;

  fhicl::ParameterSet const & getParameterSet() const;

  void putParameterSet(fhicl::ParameterSet const & newConfig);

  ServiceCache::iterator getInterfaceImpl() const;

  void makeAndCacheService(ActivityRegistry & reg) const;

  void
  createService(ActivityRegistry & reg,
                detail::ServiceStack & creationOrder) const;

  void
  convertService(WrapperBase_ptr & swb) const;

  ServiceScope serviceScope() const;

  template < typename T,
           typename = typename std::enable_if < detail::ServiceHelper<T>::scope_val != ServiceScope::PER_SCHEDULE >::type >
  T & get(ActivityRegistry & reg, detail::ServiceStack & creationOrder) const;

  template < typename T,
           typename = typename std::enable_if<detail::ServiceHelper<T>::scope_val == ServiceScope::PER_SCHEDULE>::type >
  T & get(ActivityRegistry & reg,
          detail::ServiceStack & creationOrder,
          ScheduleID sID) const;

  static void setNSchedules(size_t nSched);

private:
  static size_t & nSchedules();

  bool is_impl() const;
  bool is_interface() const;

  fhicl::ParameterSet config_;
  std::unique_ptr<detail::ServiceHelperBase> helper_;
  mutable WrapperBase_ptr service_;
  ServiceCache::iterator const interface_impl_;
};  // ServiceCacheEntry

inline
void
art::detail::ServiceCacheEntry::
forceCreation(ActivityRegistry & reg) const
{
  assert(is_impl() && "ServiceCacheEntry::forceCreation called on a service interface!");
  if (!service_) {
    makeAndCacheService(reg);
  }
}

inline
fhicl::ParameterSet const &
art::detail::ServiceCacheEntry::
getParameterSet() const
{
  return config_;
}

inline
art::detail::ServiceCache::iterator
art::detail::ServiceCacheEntry::
getInterfaceImpl() const
{
  return interface_impl_;
}

inline
void
art::detail::ServiceCacheEntry::
createService(ActivityRegistry & reg, detail::ServiceStack & creationOrder) const
{
  assert(is_impl() && "ServiceCacheEntry::createService called on a service interface!");
  // When we actually create the Service object, we have to
  // remember the order of creation.
  makeAndCacheService(reg);
  creationOrder.push(service_);
}

inline
void
art::detail::ServiceCacheEntry::
convertService(WrapperBase_ptr & swb) const
{
  assert(is_impl() && "ServiceCacheEntry::convertService called on a service interface!");
  swb = dynamic_cast<detail::ServiceInterfaceImplHelper &>(*helper_).convert(service_);
}

inline
art::ServiceScope
art::detail::ServiceCacheEntry::
serviceScope() const
{
  return helper_->scope();
}

template <typename T, typename>
inline
T &
art::detail::ServiceCacheEntry::
get(ActivityRegistry & reg, detail::ServiceStack & creationOrder) const
{
  assert(serviceScope() != ServiceScope::PER_SCHEDULE &&
         "Called wrong service get() function: need ScheduleID");
  WrapperBase_ptr swb = getService(reg, creationOrder);
  return *reinterpret_cast<T *>(dynamic_cast<detail::ServiceLGRHelper &>(*helper_).retrieve(swb));
}

template <typename T, typename>
inline
T &
art::detail::ServiceCacheEntry::
get(ActivityRegistry & reg,
    detail::ServiceStack & creationOrder,
    ScheduleID sID) const
{
  assert(serviceScope() == ServiceScope::PER_SCHEDULE &&
         "Called wrong service get() function!");
  WrapperBase_ptr swb = getService(reg, creationOrder);
  return *reinterpret_cast<T *>(dynamic_cast<detail::ServicePSRHelper &>(*helper_).retrieve(swb, sID));
}

inline
void
art::detail::ServiceCacheEntry::
setNSchedules(size_t nSched)
{
  nSchedules() = nSched;
}

inline
size_t &
art::detail::ServiceCacheEntry::
nSchedules()
{
  static size_t s_ns = 1;
  return s_ns;
}

inline
bool
art::detail::ServiceCacheEntry::
is_impl() const
{
  return !is_interface();
}

inline
bool
art::detail::ServiceCacheEntry::
is_interface() const
{
  return helper_->is_interface();
}
#endif /* art_Framework_Services_Registry_detail_ServiceCacheEntry_h */

// Local Variables:
// mode: c++
// End:
