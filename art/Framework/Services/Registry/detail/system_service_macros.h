#ifndef art_Framework_Services_Registry_detail_system_service_macros_h
#define art_Framework_Services_Registry_detail_system_service_macros_h

#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Framework/Services/Registry/detail/service_helper_macros.h"

// System service macros.

// System services require support code in Event Processor since they
// have no maker function.

#define DECLARE_ART_SYSTEM_SERVICE(svc, scope)                                 \
  namespace art::detail {                                                      \
    template <>                                                                \
    struct ServiceHelper<svc> : public ServiceImplHelper,                      \
                                public ServiceLGRHelper {                      \
      DEFINE_ART_SERVICE_TYPEID(svc)                                           \
      DEFINE_ART_SERVICE_SCOPE(scope)                                          \
      bool                                                                     \
      is_interface_impl() const override                                       \
      {                                                                        \
        return false;                                                          \
      }                                                                        \
      DEFINE_ART_SERVICE_RETRIEVER(svc)                                        \
    };                                                                         \
  }

// Note that it makes very little sense to have a system service
// implementing an interface since it has to be constructed and
// added to the service cache list with no way to define the maker
// and converter functions. One could conceivably do a second add
// for the interface, but one would have to add interface to
// ServicesManager (and ServicesManager::Cache). If we ever need it,
// it should be relatively trivial.

#endif /* art_Framework_Services_Registry_detail_system_service_macros_h */

// Local Variables:
// mode: c++
// End:
