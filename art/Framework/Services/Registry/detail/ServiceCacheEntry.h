#ifndef art_Framework_Services_Registry_detail_ServiceCacheEntry_h
#define art_Framework_Services_Registry_detail_ServiceCacheEntry_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Framework/Services/Registry/detail/ServiceStack.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

namespace art {

  class ActivityRegistry;
  class ModuleDescription;
  class ProducingServiceSignals;

  namespace detail {
    class SharedResources;

    class ServiceCacheEntry {
    public:
      ServiceCacheEntry(fhicl::ParameterSet const& pset,
                        std::unique_ptr<ServiceHelperBase>&& helper);

      ServiceCacheEntry(fhicl::ParameterSet const& pset,
                        std::unique_ptr<ServiceHelperBase>&& helper,
                        ServiceCacheEntry const& impl);

      ServiceCacheEntry(std::shared_ptr<ServiceWrapperBase> premade_service,
                        std::unique_ptr<ServiceHelperBase>&& helper);

      std::shared_ptr<ServiceWrapperBase> getService(
        ActivityRegistry& reg,
        SharedResources& resources,
        ServiceStack& creationOrder) const;

      void forceCreation(ActivityRegistry& reg,
                         SharedResources& resources) const;
      fhicl::ParameterSet const& getParameterSet() const;

      template <typename T>
      T& get(ActivityRegistry& reg,
             SharedResources& resources,
             ServiceStack& creationOrder) const;

      void registerProducts(ProductDescriptions& productsToProduce,
                            ProducingServiceSignals& signals,
                            ModuleDescription const& md);

      bool is_impl() const;
      bool is_interface() const;
      ServiceScope serviceScope() const;

    private:
      std::shared_ptr<ServiceWrapperBase> makeService(
        ActivityRegistry& reg,
        SharedResources& resources) const;

      void createService(ActivityRegistry& reg,
                         SharedResources& resources,
                         ServiceStack& creationOrder) const;

      std::shared_ptr<ServiceWrapperBase> convertService() const;

      fhicl::ParameterSet config_{};
      std::unique_ptr<ServiceHelperBase> helper_;
      mutable std::shared_ptr<ServiceWrapperBase> service_{};
      cet::exempt_ptr<ServiceCacheEntry const> const interface_impl_{nullptr};
    };

    template <typename T>
    T&
    ServiceCacheEntry::get(ActivityRegistry& reg,
                           SharedResources& resources,
                           ServiceStack& creationOrder) const
    {
      auto swb = getService(reg, resources, creationOrder);
      return *reinterpret_cast<T*>(
        dynamic_cast<ServiceLGRHelper&>(*helper_).retrieve(swb));
    }

  } // namespace detail
} // namespace art

#endif /* art_Framework_Services_Registry_detail_ServiceCacheEntry_h */

// Local Variables:
// mode: c++
// End:
