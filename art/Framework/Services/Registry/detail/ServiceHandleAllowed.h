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

#include <type_traits>

namespace art {
  namespace detail {
    template <typename T, typename = void>
    struct handle_allowed : std::true_type {
    };

    template <typename T>
    struct handle_allowed<T, std::enable_if_t<!T::service_handle_allowed>>
      : std::false_type {
    };

    template <typename T>
    bool constexpr handle_allowed_v{handle_allowed<T>::value};
  }
}

#endif /* art_Framework_Services_Registry_detail_ServiceHandleAllowed_h */

// Local Variables:
// mode: c++
// End:
