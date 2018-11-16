#ifndef art_Framework_Services_Registry_detail_helper_macros_h
#define art_Framework_Services_Registry_detail_helper_macros_h
////////////////////////////////////////////////////////////////////////
// service_macros.h
//
// Helper macros needed by the user-callable macros defined in
// ServiceMacros.h.
//
// No user-serviceable parts.
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"
#include "cetlib/compiler_macros.h"

////////////////////////////////////////////////////////////////////////
// Utility for including the service type in a static_assert
#define ART_DETAIL_STRINGIZED_VALUE(value) #value
#define ART_DETAIL_STRINGIZED_TYPE(svc) ART_DETAIL_STRINGIZED_VALUE(svc)

////////////////////////////////////////////////////////////////////////
// Describe the correct inheritance based on scope.

// Maker.
#define DEFINE_ART_SH_L_G_SCOPE_M_INTERFACE                                    \
public                                                                         \
  ServiceLGMHelper

#define DEFINE_ART_SH_LEGACY_SCOPE_M_INTERFACE                                 \
  DEFINE_ART_SH_L_G_SCOPE_M_INTERFACE

#define DEFINE_ART_SH_GLOBAL_SCOPE_M_INTERFACE                                 \
  DEFINE_ART_SH_L_G_SCOPE_M_INTERFACE

#define DEFINE_ART_SH_PER_SCHEDULE_SCOPE_M_INTERFACE                           \
public                                                                         \
  ServicePSMHelper

#define DEFINE_ART_SH_SCOPE_M_INTERFACE(scopeArg)                              \
  DEFINE_ART_SH_##scopeArg##_SCOPE_M_INTERFACE

// Retriever.
#define DEFINE_ART_SH_L_G_SCOPE_R_INTERFACE                                    \
public                                                                         \
  ServiceLGRHelper

#define DEFINE_ART_SH_LEGACY_SCOPE_R_INTERFACE                                 \
  DEFINE_ART_SH_L_G_SCOPE_R_INTERFACE

#define DEFINE_ART_SH_GLOBAL_SCOPE_R_INTERFACE                                 \
  DEFINE_ART_SH_L_G_SCOPE_R_INTERFACE

#define DEFINE_ART_SH_PER_SCHEDULE_SCOPE_R_INTERFACE                           \
public                                                                         \
  ServicePSRHelper

#define DEFINE_ART_SH_SCOPE_R_INTERFACE(scopeArg)                              \
  DEFINE_ART_SH_##scopeArg##_SCOPE_R_INTERFACE

////////////////////////////////////////////////////////////////////////
// Define a member function returning the typeid of the service.
#define DEFINE_ART_SERVICE_TYPEID(svc)                                         \
  art::TypeID get_typeid() const override { return TypeID{typeid(svc)}; }

////////////////////////////////////////////////////////////////////////
// Define a member function to return the scope of the service, and a
// static to provide compile-time help when we have the concrete helper
// class instead of a base.
#define DEFINE_ART_SERVICE_SCOPE(scopeArg)                                     \
  ServiceScope scope() const override { return ServiceScope::scopeArg; }       \
  static constexpr ServiceScope scope_val                                      \
    [[gnu::unused]]{ServiceScope::scopeArg};

////////////////////////////////////////////////////////////////////////
// Define a member function to retrieve the desired service.

// Legacy and global services.
#define DEFINE_ART_L_G_SERVICE_RETRIEVER(svc, scopeArg)                        \
  void* retrieve(std::shared_ptr<ServiceWrapperBase>& swb)                     \
    const final override                                                       \
  {                                                                            \
    return &dynamic_cast<ServiceWrapper<svc, ServiceScope::scopeArg>*>(        \
              swb.get())                                                       \
              ->get();                                                         \
  }

// Legacy service.
#define DEFINE_ART_LEGACY_SERVICE_RETRIEVER(svc)                               \
  DEFINE_ART_L_G_SERVICE_RETRIEVER(svc, LEGACY)

// Global service.
#define DEFINE_ART_GLOBAL_SERVICE_RETRIEVER(svc)                               \
  DEFINE_ART_L_G_SERVICE_RETRIEVER(svc, GLOBAL)

// Per-schedule service.
#define DEFINE_ART_PER_SCHEDULE_SERVICE_RETRIEVER(svc)                         \
  void* retrieve(std::shared_ptr<ServiceWrapperBase>& swb,                     \
                 ScheduleID const sID) const final override                    \
  {                                                                            \
    return &dynamic_cast<ServiceWrapper<svc, ServiceScope::PER_SCHEDULE>*>(    \
              swb.get())                                                       \
              ->get(sID);                                                      \
  }

////////////////////////////////////////////////////////////////////////
// Define a member function to make the desired service.

// Legacy and global services.
#define DEFINE_ART_L_G_SERVICE_MAKER(svc, scopeArg)                            \
  std::unique_ptr<ServiceWrapperBase> make(fhicl::ParameterSet const& cfg,     \
                                           ActivityRegistry& reg)              \
    const final override                                                       \
  {                                                                            \
    return std::make_unique<ServiceWrapper<svc, ServiceScope::scopeArg>>(cfg,  \
                                                                         reg); \
  }

// Legacy services.
#define DEFINE_ART_LEGACY_SERVICE_MAKER(svc)                                   \
  DEFINE_ART_L_G_SERVICE_MAKER(svc, LEGACY)

// Global services.
#define DEFINE_ART_GLOBAL_SERVICE_MAKER(svc)                                   \
  DEFINE_ART_L_G_SERVICE_MAKER(svc, GLOBAL)

// Per-schedule services.
#define DEFINE_ART_PER_SCHEDULE_SERVICE_MAKER(svc)                             \
  std::unique_ptr<ServiceWrapperBase> make(                                    \
    fhicl::ParameterSet const& cfg, ActivityRegistry& reg, size_t nSchedules)  \
    const final override                                                       \
  {                                                                            \
    return std::make_unique<ServiceWrapper<svc, ServiceScope::PER_SCHEDULE>>(  \
      cfg, reg, nSchedules);                                                   \
  }

// CreateHelper.
#define DEFINE_ART_SERVICE_HELPER_CREATE(svc)                                  \
  static std::unique_ptr<art::detail::ServiceHelperBase> createHelper()        \
  {                                                                            \
    return std::make_unique<art::detail::ServiceHelper<svc>>();                \
  }

////////////////////////////////////////////////////////////////////////
// Detail macros invoked by user-visible macros.

////////////////////////////////////
// Multi-schedule-aware service declarations.

// Declare a multi-schedule-aware service.
#define DECLARE_ART_SERVICE_DETAIL(svc, scopeArg)                              \
  namespace art {                                                              \
    namespace detail {                                                         \
      template <>                                                              \
      struct ServiceHelper<svc> : public ServiceImplHelper,                    \
                                  DEFINE_ART_SH_SCOPE_M_INTERFACE(scopeArg),   \
                                  DEFINE_ART_SH_SCOPE_R_INTERFACE(scopeArg) {  \
        DEFINE_ART_SERVICE_TYPEID(svc)                                         \
        DEFINE_ART_SERVICE_SCOPE(scopeArg)                                     \
        bool                                                                   \
        is_interface_impl() const override                                     \
        {                                                                      \
          return false;                                                        \
        }                                                                      \
        DEFINE_ART_##scopeArg##_SERVICE_MAKER(svc) DEFINE_ART_##scopeArg       \
          ##_SERVICE_RETRIEVER(svc)                                            \
      };                                                                       \
    }                                                                          \
  }

// Declare an interface for services.
#define DECLARE_ART_SERVICE_INTERFACE_DETAIL(iface, scopeArg)                  \
  namespace art {                                                              \
    namespace detail {                                                         \
      template <>                                                              \
      struct ServiceHelper<iface>                                              \
        : public ServiceInterfaceHelper,                                       \
          DEFINE_ART_SH_SCOPE_R_INTERFACE(scopeArg) {                          \
        DEFINE_ART_SERVICE_TYPEID(iface)                                       \
        DEFINE_ART_SERVICE_SCOPE(scopeArg)                                     \
        DEFINE_ART_##scopeArg##_SERVICE_RETRIEVER(iface)                       \
      };                                                                       \
    }                                                                          \
  }

// Define a multi-schedule-aware service implementing an interface.
#define DECLARE_ART_SERVICE_INTERFACE_IMPL_DETAIL(svc, iface, scopeArg)        \
  namespace art {                                                              \
    namespace detail {                                                         \
      template <>                                                              \
      struct ServiceHelper<svc> : public ServiceInterfaceImplHelper,           \
                                  DEFINE_ART_SH_SCOPE_M_INTERFACE(scopeArg),   \
                                  DEFINE_ART_SH_SCOPE_R_INTERFACE(scopeArg) {  \
        DEFINE_ART_SERVICE_TYPEID(svc)                                         \
        DEFINE_ART_SERVICE_SCOPE(scopeArg)                                     \
        DEFINE_ART_##scopeArg##_SERVICE_MAKER(svc) DEFINE_ART_##scopeArg       \
          ##_SERVICE_RETRIEVER(svc) art::TypeID                                \
          get_interface_typeid() const final override                          \
        {                                                                      \
          return TypeID{typeid(iface)};                                        \
        }                                                                      \
        std::unique_ptr<ServiceWrapperBase>                                    \
        convert(                                                               \
          std::shared_ptr<ServiceWrapperBase> const& swb) const final override \
        {                                                                      \
          return std::unique_ptr<art::detail::ServiceWrapperBase>(             \
            static_cast<art::detail::ServiceWrapperBase*>(                     \
              std::dynamic_pointer_cast<                                       \
                ServiceWrapper<svc, ServiceScope::scopeArg>>(swb)              \
                ->getAs<iface>()));                                            \
        }                                                                      \
        static_assert(scope_val ==                                             \
                        ServiceHelper<iface>::scope_val, /* Safety check */    \
                      "Scope mismatch between interface " #iface               \
                      " and implementation " #svc);                            \
      };                                                                       \
    }                                                                          \
  }

////////////////////////////////////////////////////////////////////////
// Declare a system service (requires support code in Event Processor)
// since system services have no maker function.
#define DECLARE_ART_SYSTEM_SERVICE_DETAIL(svc, scopeArg)                       \
  namespace art {                                                              \
    namespace detail {                                                         \
      template <>                                                              \
      struct ServiceHelper<svc> : public ServiceImplHelper,                    \
                                  DEFINE_ART_SH_SCOPE_R_INTERFACE(scopeArg) {  \
        DEFINE_ART_SERVICE_TYPEID(svc)                                         \
        DEFINE_ART_SERVICE_SCOPE(scopeArg)                                     \
        bool                                                                   \
        is_interface_impl() const override                                     \
        {                                                                      \
          return false;                                                        \
        }                                                                      \
        DEFINE_ART_##scopeArg##_SERVICE_RETRIEVER(svc)                         \
      };                                                                       \
    }                                                                          \
  }

////////////////////////////////////////////////////////////////////////
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
