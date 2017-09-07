#ifndef art_Framework_Services_Registry_detail_ServiceCacheEntry_h
#define art_Framework_Services_Registry_detail_ServiceCacheEntry_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceStack.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"
#include "fhiclcpp/ParameterSet.h"

#include <cassert>
#include <memory>

namespace art {

class ActivityRegistry;

namespace detail {

class ServiceCacheEntry {

public: // MEMBER FUNCTIONS -- Special Member Functions

  ServiceCacheEntry(fhicl::ParameterSet const& pset, std::unique_ptr<ServiceHelperBase>&& helper);

  ServiceCacheEntry(fhicl::ParameterSet const& pset, std::unique_ptr<ServiceHelperBase>&& helper, ServiceCacheEntry const& impl);

  ServiceCacheEntry(std::shared_ptr<ServiceWrapperBase> premade_service, std::unique_ptr<ServiceHelperBase>&& helper);

public: // MEMBER FUNCTIONS -- Public API

  std::shared_ptr<ServiceWrapperBase>
  getService(art::ActivityRegistry& reg, ServiceStack& creationOrder) const;

  void
  forceCreation(art::ActivityRegistry& reg) const;

  fhicl::ParameterSet const&
  getParameterSet() const;

  template <typename T>
  T&
  get(art::ActivityRegistry& reg, ServiceStack& creationOrder) const;

private: // MEMBER FUNCTIONS -- Implementation details

  void
  makeAndCacheService(art::ActivityRegistry& reg) const;

  void
  createService(art::ActivityRegistry& reg, ServiceStack& creationOrder) const;

  void
  convertService(std::shared_ptr<ServiceWrapperBase>& swb) const;

  ServiceScope
  serviceScope() const;

  bool
  is_impl() const;

  bool
  is_interface() const;

private: // MEMBER DATA

  fhicl::ParameterSet
  config_{};

  std::unique_ptr<ServiceHelperBase>
  helper_;

  mutable
  std::shared_ptr<ServiceWrapperBase>
  service_{};

  ServiceCacheEntry const* const
  interface_impl_{nullptr};

};

template <typename T>
T&
ServiceCacheEntry::
get(art::ActivityRegistry& reg, ServiceStack& creationOrder) const
{
  std::shared_ptr<ServiceWrapperBase> swb = getService(reg, creationOrder);
  return *reinterpret_cast<T*>(dynamic_cast<ServiceLGRHelper&>(*helper_).retrieve(swb));
}

} // namespace detail
} // namespace art

#endif /* art_Framework_Services_Registry_detail_ServiceCacheEntry_h */

// Local Variables:
// mode: c++
// End:
