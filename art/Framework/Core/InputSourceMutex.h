#ifndef art_Framework_Core_InputSourceMutex_h
#define art_Framework_Core_InputSourceMutex_h
// vim: set sw=2 expandtab :

#include "hep_concurrency/RecursiveMutex.h"

namespace art {

  class InputSourceMutexSentry {
  public:
    static hep::concurrency::RecursiveMutex* startup();
    static void shutdown();

    ~InputSourceMutexSentry() noexcept;
    InputSourceMutexSentry();
    InputSourceMutexSentry(InputSourceMutexSentry const&) = delete;
    InputSourceMutexSentry& operator=(InputSourceMutexSentry const&) = delete;

  private:
    friend class AutoInputSourceMutexSentryShutdown;
    static hep::concurrency::RecursiveMutex* inputSourceMutex_;
    hep::concurrency::RecursiveMutexSentry sentry_;
  };

} // namespace art

#endif /* art_Framework_Core_InputSourceMutex_h */

// Local Variables:
// mode: c++
// End:
