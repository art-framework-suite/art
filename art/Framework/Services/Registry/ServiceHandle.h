#ifndef art_Framework_Services_Registry_ServiceHandle_h
#define art_Framework_Services_Registry_ServiceHandle_h
// vim: set sw=2 expandtab :

//
// ServiceHandle
//
// Smart pointer used to give easy access to Services.
//
// Note invocation only requires one template argument, but the
// constructor will require zero or one arguments depending on the scope
// of the service (LEGACY, GLOBAL).
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
//

#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceHandleAllowed.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Utilities/ScheduleID.h"

#include <type_traits>

namespace art {

  template <typename T,
            ServiceScope SCOPE =
              detail::ServiceHelper<std::remove_const_t<T>>::scope_val>
  class ServiceHandle {
  public:
    static_assert(
      detail::handle_allowed_v<T>,
      "\n\nart-error: You cannot create a ServiceHandle for this type.\n"
      "           Please contact artists@fnal.gov for guidance.\n");

    ServiceHandle() try : instance {
      &ServiceRegistry::instance().get<std::remove_const_t<T>>()
    }
    {
    }
    catch (Exception const& x)
    {
      throw Exception(errors::ServiceNotFound)
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

} // namespace art

#endif /* art_Framework_Services_Registry_ServiceHandle_h */

// Local Variables:
// mode: c++
// End:
