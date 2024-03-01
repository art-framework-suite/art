#ifndef art_Framework_Services_Registry_detail_ServiceHandleAllowed_h
#define art_Framework_Services_Registry_detail_ServiceHandleAllowed_h

// ======================================================================
// ServiceHandleAllowed is a compile-time trait that checks if a
// 'service_handle_allowed' constexpr boolean member is present in a
// service's class definition *and* has been set to false.  If set to
// 'false', then the 'handle_allowed_v<T>' value is 'false'.  If a
// 'service_handle_allowed' variable is not present in a service's
// definition, then the 'handle_allowed_v<T>' value is 'true'.
// ======================================================================

#include <concepts>
#include <type_traits>

namespace art::detail {
  template <typename T>
  concept handle_not_allowed = requires {
                                 {
                                   T::service_handle_allowed
                                   } -> std::same_as<bool>;
                               } && !
  T::service_handle_allowed;

  template <typename T>
  concept handle_allowed = !
  handle_not_allowed<T>;

  template <typename T>
  concept handle_allowed_v = handle_allowed<T>;
}

#endif /* art_Framework_Services_Registry_detail_ServiceHandleAllowed_h */

// Local Variables:
// mode: c++
// End:
