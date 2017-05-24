#ifndef art_Framework_Services_Registry_ServiceHandle_h
#define art_Framework_Services_Registry_ServiceHandle_h

// ======================================================================
// ServiceHandle
//
// Smart pointer used to give easy access to Services.
//
// Note invocation only requires one template argument, but the
// constructor will require zero or one arguments depending on the scope
// of the service (LEGACY, GLOBAL or PER_SCHEDULE).
//
// Technical notes:
//
//    Since ServiceHelper<T> instantiations correspond to
//    specializations created by macro calls, only a template argument
//    'T' that is non-const qualified will match any specialization.
//    However, const-only access to a service can be provided via
//    ServiceHandle<MyService const> as long as the const-ness of the
//    template argument is stripped (via std::remove_const_t<T>)
//    before serving as an argument to ServiceHelper.
// ======================================================================

#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Utilities/ScheduleID.h"

#include <type_traits>

namespace art {
  template <typename T, ServiceScope SCOPE = art::detail::ServiceHelper<std::remove_const_t<T>>::scope_val> class ServiceHandle;
  template <typename T> class ServiceHandle<T, art::ServiceScope::PER_SCHEDULE>;
}

// General template.
template <typename T, art::ServiceScope SCOPE>
class art::ServiceHandle {
public:

  ServiceHandle()
    : instance{&ServiceRegistry::instance().get<std::remove_const_t<T>>()}
  {}

  T* operator->() const { return instance; }
  T& operator*() const { return *instance; }
  T* get() const { return instance; }

private:
  T* instance;
};

// Per-schedule template. SFINAE wouldn't work here.
template <typename T>
class art::ServiceHandle<T, art::ServiceScope::PER_SCHEDULE> {
public:

  ServiceHandle(ScheduleID const sID)
    : instance{&ServiceRegistry::instance().get<std::remove_const_t<T>>(sID)}
  {}

  T* operator->() const { return instance; }
  T& operator*() const { return *instance; }
  T* get() const { return instance; }

private:
  T* instance;
};

// ======================================================================

#endif /* art_Framework_Services_Registry_ServiceHandle_h */

// Local Variables:
// mode: c++
// End:
