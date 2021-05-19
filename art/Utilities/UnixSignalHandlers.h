#ifndef art_Utilities_UnixSignalHandlers_h
#define art_Utilities_UnixSignalHandlers_h
// vim: set sw=2 expandtab :

#include <atomic>

namespace art {

  extern std::atomic<int> shutdown_flag;
  void setupSignals(bool want_sigint_enabled);

} // namespace art

#endif /* art_Utilities_UnixSignalHandlers_h */

// Local Variables:
// mode: c++
// End:
