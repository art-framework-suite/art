#ifndef art_Framework_Services_Registry_ServiceScope_h
#define art_Framework_Services_Registry_ServiceScope_h
// vim: set sw=2 expandtab :

namespace art {

  enum class ServiceScope { LEGACY, SHARED };

  constexpr bool
  is_legacy(ServiceScope const scope) noexcept
  {
    return scope == ServiceScope::LEGACY;
  }
  constexpr bool
  is_shared(ServiceScope const scope) noexcept
  {
    return scope == ServiceScope::SHARED;
  }

} // namespace art

#endif /* art_Framework_Services_Registry_ServiceScope_h */

// Local Variables:
// mode: c++
// End:
