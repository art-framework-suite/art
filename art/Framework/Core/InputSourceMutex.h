#ifndef art_Framework_Core_InputSourceMutex_h
#define art_Framework_Core_InputSourceMutex_h
// vim: set sw=2 expandtab :

#include <mutex>

namespace art {

  class InputSourceMutexSentry {
  public:
    ~InputSourceMutexSentry() noexcept;
    InputSourceMutexSentry();

  private:
    static std::recursive_mutex inputSourceMutex_;
  };

} // namespace art

#endif /* art_Framework_Core_InputSourceMutex_h */

// Local Variables:
// mode: c++
// End:
