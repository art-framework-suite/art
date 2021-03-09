#ifndef art_Framework_Services_Registry_detail_service_helper_macros_h
#define art_Framework_Services_Registry_detail_service_helper_macros_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>

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

#endif /* art_Framework_Services_Registry_detail_service_helper_macros_h */

// Local Variables:
// mode: c++
// End:
