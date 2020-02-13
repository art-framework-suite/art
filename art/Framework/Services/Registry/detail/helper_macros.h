#ifndef art_Framework_Services_Registry_detail_helper_macros_h
#define art_Framework_Services_Registry_detail_helper_macros_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceHandleAllowed.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"
#include "art/Framework/Services/Registry/detail/ensure_only_one_thread.h"
#include "art/Utilities/SharedResourcesRegistry.h"

////////////////////////////////////////////////////////////////////////
// Utility for including the service type in a static_assert
#define ART_DETAIL_STRINGIZED_VALUE(value) #value
#define ART_DETAIL_STRINGIZED_TYPE(svc) ART_DETAIL_STRINGIZED_VALUE(svc)

// Define a member function returning the typeid of the service.
#define DEFINE_ART_SERVICE_TYPEID(svc)                                         \
  art::TypeID get_typeid() const override { return TypeID{typeid(svc)}; }

// Define a member function to return the scope of the service, and a
// static to provide compile-time help when we have the concrete helper
// class instead of a base.
#define DEFINE_ART_SERVICE_SCOPE(scopeArg)                                     \
  ServiceScope scope() const override { return scope_val; }                    \
  static constexpr ServiceScope scope_val{ServiceScope::scopeArg};

#define DEFINE_ART_SERVICE_RETRIEVER(svc)                                      \
  void* retrieve(std::shared_ptr<ServiceWrapperBase>& swb)                     \
    const final override                                                       \
  {                                                                            \
    return &std::dynamic_pointer_cast<ServiceWrapper<svc>>(swb)->get();        \
  }

#define DEFINE_ART_SERVICE_MAKER(svc, scopeArg)                                \
  std::unique_ptr<ServiceWrapperBase> make(fhicl::ParameterSet const& pset,    \
                                           ActivityRegistry& reg)              \
    const final override                                                       \
  {                                                                            \
    if constexpr (is_shared(ServiceScope::scopeArg) &&                         \
                  detail::handle_allowed_v<svc>) {                             \
      SharedResourcesRegistry::instance()->registerSharedResource(             \
        SharedResource<svc>);                                                  \
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
      static_assert(scope_val == ServiceHelper<iface>::scope_val,              \
                    "Scope mismatch between interface " #iface                 \
                    " and implementation " #svc);                              \
    };                                                                         \
  }

// Declare a system service (requires support code in Event Processor)
// since system services have no maker function.
#define ART_DETAIL_DECLARE_SYSTEM_SERVICE(svc, scopeArg)                       \
  namespace art::detail {                                                      \
    template <>                                                                \
    struct ServiceHelper<svc> : public ServiceImplHelper,                      \
                                public ServiceLGRHelper {                      \
      DEFINE_ART_SERVICE_TYPEID(svc)                                           \
      DEFINE_ART_SERVICE_SCOPE(scopeArg)                                       \
      bool                                                                     \
      is_interface_impl() const override                                       \
      {                                                                        \
        return false;                                                          \
      }                                                                        \
      DEFINE_ART_SERVICE_RETRIEVER(svc)                                        \
    };                                                                         \
  }

// Define the C-linkage function to  create the helper.
#define DEFINE_ART_SH_CREATE(svc) DEFINE_ART_SH_CREATE_DETAIL(svc, service)

#define DEFINE_ART_SIH_CREATE(svc) DEFINE_ART_SH_CREATE_DETAIL(svc, iface)

#define DEFINE_ART_SH_CREATE_DETAIL(svc, type)                                 \
  EXTERN_C_FUNC_DECLARE_START                                                  \
  std::unique_ptr<art::detail::ServiceHelperBase> create_##type##_helper()     \
  {                                                                            \
    return std::make_unique<art::detail::ServiceHelper<svc>>();                \
  }                                                                            \
  EXTERN_C_FUNC_DECLARE_END

#endif /* art_Framework_Services_Registry_detail_helper_macros_h */

// Local Variables:
// mode: c++
// End:
