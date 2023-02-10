#include "art/Framework/Core/InputSourceMutex.h"
// vim: set sw=2 expandtab :

namespace art {
  std::recursive_mutex InputSourceMutexSentry::inputSourceMutex_{};

  InputSourceMutexSentry::~InputSourceMutexSentry() noexcept
  {
    inputSourceMutex_.unlock();
  }

  InputSourceMutexSentry::InputSourceMutexSentry()
  {
    inputSourceMutex_.lock();
  }
} // namespace art
