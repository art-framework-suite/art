#ifndef art_Framework_Services_Registry_ServiceScope_h
#define art_Framework_Services_Registry_ServiceScope_h
// vim: set sw=2 expandtab :

#define GLOBAL_DEPRECATION_MSG                                                 \
  "\n\nart warning: GLOBAL services are now deprecated; please replace the\n"  \
  "             GLOBAL scope name with SHARED.\n\n"

namespace art {

  enum class ServiceScope {
    LEGACY,
    SHARED,
    GLOBAL [[deprecated(GLOBAL_DEPRECATION_MSG)]] = SHARED,
    PER_SCHEDULE
  };

} // namespace art

#undef GLOBAL_DEPRECATION_MSG

#endif /* art_Framework_Services_Registry_ServiceScope_h */

// Local Variables:
// mode: c++
// End:
