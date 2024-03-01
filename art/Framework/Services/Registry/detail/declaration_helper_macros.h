#ifndef art_Framework_Services_Registry_detail_declaration_helper_macros_h
#define art_Framework_Services_Registry_detail_declaration_helper_macros_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceHandleAllowed.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"
#include "art/Framework/Services/Registry/detail/ensure_only_one_thread.h"
#include "art/Framework/Services/Registry/detail/service_helper_macros.h"
#include "art/Utilities/SharedResource.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>

////////////////////////////////////////////////////////////////////////
// Utility for including the service type in a static_assert
#define ART_DETAIL_STRINGIZED_VALUE(value) #value
#define ART_DETAIL_STRINGIZED_TYPE(svc) ART_DETAIL_STRINGIZED_VALUE(svc)

#define DEFINE_ART_SERVICE_MAKER(svc, scopeArg)                                \
  std::unique_ptr<ServiceWrapperBase> make(fhicl::ParameterSet const& pset,    \
                                           ActivityRegistry& reg,              \
                                           detail::SharedResources& resources) \
    const final override                                                       \
  {                                                                            \
    if constexpr (is_shared(ServiceScope::scopeArg) &&                         \
                  detail::handle_allowed<svc>) {                             \
      resources.registerSharedResource(SharedResource<svc>);                   \
    } else if constexpr (is_legacy(ServiceScope::scopeArg)) {                  \
      ensure_only_one_thread(pset);                                            \
    }                                                                          \
    return std::make_unique<ServiceWrapper<svc>>(pset, reg);                   \
  }

// CreateHelper.
#define DEFINE_ART_SERVICE_HELPER_CREATE(svc)                                  \
  static std::unique_ptr<ServiceHelperBase> createHelper()                     \
  {                                                                            \
    return std::make_unique<ServiceHelper<svc>>();                             \
  }

// Declare a service with scope.
#define ART_DETAIL_DECLARE_SERVICE(svc, scopeArg)                              \
  namespace art::detail {                                                      \
    template <>                                                                \
    struct ServiceHelper<svc> : public ServiceImplHelper,                      \
                                public ServiceLGMHelper,                       \
                                public ServiceLGRHelper {                      \
      DEFINE_ART_SERVICE_TYPEID(svc)                                           \
      DEFINE_ART_SERVICE_SCOPE(scopeArg)                                       \
      DEFINE_ART_SERVICE_RETRIEVER(svc)                                        \
      DEFINE_ART_SERVICE_MAKER(svc, scopeArg)                                  \
      bool                                                                     \
      is_interface_impl() const override                                       \
      {                                                                        \
        return false;                                                          \
      }                                                                        \
    };                                                                         \
  }

// Declare an interface for a service with scope.
#define ART_DETAIL_DECLARE_SERVICE_INTERFACE(iface, scopeArg)                  \
  namespace art::detail {                                                      \
    template <>                                                                \
    struct ServiceHelper<iface> : public ServiceInterfaceHelper,               \
                                  public ServiceLGRHelper {                    \
      DEFINE_ART_SERVICE_TYPEID(iface)                                         \
      DEFINE_ART_SERVICE_SCOPE(scopeArg)                                       \
      DEFINE_ART_SERVICE_RETRIEVER(iface)                                      \
    };                                                                         \
  }

// Define a service with scope implementing an interface.
#define ART_DETAIL_DECLARE_SERVICE_INTERFACE_IMPL(svc, iface, scopeArg)        \
  namespace art::detail {                                                      \
    template <>                                                                \
    struct ServiceHelper<svc> : public ServiceInterfaceImplHelper,             \
                                public ServiceLGMHelper,                       \
                                public ServiceLGRHelper {                      \
      DEFINE_ART_SERVICE_TYPEID(svc)                                           \
      DEFINE_ART_SERVICE_SCOPE(scopeArg)                                       \
      DEFINE_ART_SERVICE_RETRIEVER(svc)                                        \
      DEFINE_ART_SERVICE_MAKER(svc, scopeArg)                                  \
      art::TypeID                                                              \
      get_interface_typeid() const final override                              \
      {                                                                        \
        return TypeID{typeid(iface)};                                          \
      }                                                                        \
      std::unique_ptr<ServiceWrapperBase>                                      \
      convert(                                                                 \
        std::shared_ptr<ServiceWrapperBase> const& swb) const final override   \
      {                                                                        \
        return std::dynamic_pointer_cast<ServiceWrapper<svc>>(swb)             \
          ->getAs<iface>();                                                    \
      }                                                                        \
      static_assert(is_shared(ServiceHelper<iface>::scope_val) ||              \
                      is_legacy(ServiceHelper<svc>::scope_val),                \
                    "\n\nart error: An implementation that inherits from a "   \
                    "LEGACY interface\n"                                       \
                    "           must be a LEGACY service\n\n");                \
    };                                                                         \
  }

#endif /* art_Framework_Services_Registry_detail_declaration_helper_macros_h   \
        */

// Local Variables:
// mode: c++
// End:
