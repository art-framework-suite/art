#include "art/Framework/Core/InputSourceMutex.h"
// vim: set sw=2 expandtab :

#include "canvas/Utilities/Exception.h"
#include "hep_concurrency/RecursiveMutex.h"
#include "hep_concurrency/tsan.h"

using namespace std;
using namespace hep::concurrency;

namespace art {
  RecursiveMutex* InputSourceMutexSentry::inputSourceMutex_{
    new RecursiveMutex{"art::inputSourceMutex_"}};

  RecursiveMutex*
  InputSourceMutexSentry::startup()
  {
    static mutex guard_mutex;
    lock_guard sentry{guard_mutex};
    inputSourceMutex_ = new RecursiveMutex{"art::inputSourceMutex_"};
    return inputSourceMutex_;
  }

  void
  InputSourceMutexSentry::shutdown()
  {
    ANNOTATE_THREAD_IGNORE_BEGIN;
    delete inputSourceMutex_;
    inputSourceMutex_ = nullptr;
    ANNOTATE_THREAD_IGNORE_END;
  }

  class AutoInputSourceMutexSentryShutdown {
  public:
    AutoInputSourceMutexSentryShutdown() noexcept = default;
    ~AutoInputSourceMutexSentryShutdown()
    {
      InputSourceMutexSentry::shutdown();
    }
    AutoInputSourceMutexSentryShutdown(
      AutoInputSourceMutexSentryShutdown const&) = delete;
    AutoInputSourceMutexSentryShutdown(AutoInputSourceMutexSentryShutdown&&) =
      delete;
    AutoInputSourceMutexSentryShutdown& operator=(
      AutoInputSourceMutexSentryShutdown const&) = delete;
    AutoInputSourceMutexSentryShutdown& operator=(
      AutoInputSourceMutexSentryShutdown&&) = delete;
  };

  // The inputSourceMutex_ will be destroyed at global destruction
  // time if it has not yet been cleanedup.  This is to guard against
  // libraries that do not follow the rules.
  AutoInputSourceMutexSentryShutdown autoInputSourceMutexSentryShutdown;

  InputSourceMutexSentry::~InputSourceMutexSentry() noexcept = default;

  InputSourceMutexSentry::InputSourceMutexSentry()
    : sentry_{(inputSourceMutex_ == nullptr) ? *startup() : *inputSourceMutex_,
              __func__}
  {}

} // namespace art
