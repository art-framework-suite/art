#ifndef art_Framework_Services_Registry_detail_ServiceHelper_h
#define art_Framework_Services_Registry_detail_ServiceHelper_h
// vim: set sw=2 expandtab :

//
// The ServiceHelper class and associated interface classes to simplify
// the service cache in the face of pre-made services,
// and services implementing interfaces.
//
//   LG = Legacy/Global  M = make  R = retrieve
//

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>

namespace art {

  class ActivityRegistry;

  namespace detail {

    struct ServiceHelperBase;
    struct ServiceImplHelper;
    struct ServiceInterfaceImplHelper;
    struct ServiceInterfaceHelper;
    struct ServiceLGMHelper;
    struct ServiceLGRHelper;
    template <typename SERVICE>
    struct ServiceHelper;

    // Base class. Note virtual inheritance below.
    struct ServiceHelperBase {
      virtual ~ServiceHelperBase() noexcept = default;
      virtual TypeID get_typeid() const = 0;
      virtual ServiceScope scope() const = 0;
      virtual bool is_interface() const = 0;
      virtual bool is_interface_impl() const = 0;
    };

    // For a "real" service (not an interface).
    struct ServiceImplHelper : public virtual ServiceHelperBase {
      bool
      is_interface() const override
      {
        return false;
      }
    };

    // For a service implementing an interface.
    struct ServiceInterfaceImplHelper : public ServiceImplHelper {
      bool
      is_interface_impl() const final override
      {
        return true;
      }
      virtual TypeID get_interface_typeid() const = 0;
      virtual std::unique_ptr<ServiceWrapperBase> convert(
        std::shared_ptr<ServiceWrapperBase> const& swb) const = 0;
    };

    // For the service interface itself.
    struct ServiceInterfaceHelper : public virtual ServiceHelperBase {
      bool
      is_interface() const final override
      {
        return true;
      }
      bool
      is_interface_impl() const final override
      {
        return false;
      }
    };

    // LegacyOrGlobalMaker
    // For a global or legacy service
    struct ServiceLGMHelper {
      virtual ~ServiceLGMHelper() noexcept = default;
      virtual std::unique_ptr<ServiceWrapperBase> make(
        fhicl::ParameterSet const& cfg,
        ActivityRegistry& reg) const = 0;
    };

    // LegacyOrGlobalRetriever
    struct ServiceLGRHelper {
      virtual ~ServiceLGRHelper() noexcept = default;
      virtual void* retrieve(
        std::shared_ptr<ServiceWrapperBase>& swb) const = 0;
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Services_Registry_detail_ServiceHelper_h */

// Local Variables:
// mode: c++
// End:
