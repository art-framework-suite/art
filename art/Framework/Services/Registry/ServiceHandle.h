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

  template <typename T, typename = void>
  struct handle_allowed : std::true_type {
  };

  template <typename T>
  struct handle_allowed<T, std::enable_if_t<!T::service_handle_allowed>>
    : std::false_type {
  };

  template <typename T>
  bool constexpr handle_allowed_v{handle_allowed<T>::value};

  template <typename T,
            ServiceScope SCOPE =
              art::detail::ServiceHelper<std::remove_const_t<T>>::scope_val>
  class ServiceHandle;
  template <typename T>
  class ServiceHandle<T, art::ServiceScope::PER_SCHEDULE>;
}

// General template.
template <typename T, art::ServiceScope SCOPE>
class art::ServiceHandle {
public:
  static_assert(
    handle_allowed_v<T>,
    "\n\nart-error: You cannot create a ServiceHandle for this type.\n"
    "           Please contact artists@fnal.gov for guidance.\n");

  ServiceHandle() try : instance {
    &ServiceRegistry::instance().get<std::remove_const_t<T>>()
  }
  {
  }
  catch (art::Exception const& x)
  {
    throw art::Exception(art::errors::ServiceNotFound)
      << "Unable to create ServiceHandle.\n"
      << "Perhaps the FHiCL configuration does not specify the necessary "
         "service?\n"
      << "The class of the service is noted below...\n"
      << x;
  }

  T* operator->() const { return instance; }
  T& operator*() const { return *instance; }
  T*
  get() const
  {
    return instance;
  }

private:
  T* instance;
};

// Per-schedule template. SFINAE wouldn't work here.
template <typename T>
class art::ServiceHandle<T, art::ServiceScope::PER_SCHEDULE> {
public:
  explicit ServiceHandle(ScheduleID const sID) try : instance {
    &ServiceRegistry::instance().get<std::remove_const_t<T>>(sID)
  }
  {
  }
  catch (art::Exception const& x)
  {
    throw art::Exception(art::errors::ServiceNotFound)
      << "Unable to create ServiceHandle.\n"
      << "Perhaps the FHiCL configuration does not specify the necessary "
         "service?\n"
      << "The class of the service is noted below...\n"
      << x;
  }

  T* operator->() const { return instance; }
  T& operator*() const { return *instance; }
  T*
  get() const
  {
    return instance;
  }

private:
  T* instance;
};

// ======================================================================

#endif /* art_Framework_Services_Registry_ServiceHandle_h */

// Local Variables:
// mode: c++
// End:
