#ifndef art_Framework_Core_SharedException_h
#define art_Framework_Core_SharedException_h

#include <atomic>
#include <exception>
#include <thread>

// ============================================================================
// The SharedException class exists to facilitate the communication of
// exceptions produced on worker threads to the main thread of
// execution.  The pattern is that any thread may store its exception
// in an object of this type; however, the exception must be thrown
// from the same thread in which the SharedException object has been
// created.
// ============================================================================

namespace art {
  class SharedException {
  public:
    void
    store(std::exception_ptr ex_ptr)
    {
      bool expected = false;
      if (cachedExceptionStored_.compare_exchange_strong(expected, true)) {
        // Put the exception where the main thread can get at it.
        cachedException_ = std::move(ex_ptr);
      }
    }

    void
    store_current()
    {
      store(std::current_exception());
    }

    template <typename T, typename... Args>
    void
    store(Args&&... args)
    {
      store(std::make_exception_ptr(T{std::forward<Args>(args)...}));
    }

    void
    throw_if_stored_exception()
    {
      assert(std::this_thread::get_id() == ownerThread_);
      if (cachedExceptionStored_.load()) {
        std::rethrow_exception(cachedException_);
      }
    }

  private:
    std::thread::id const ownerThread_{std::this_thread::get_id()};
    std::atomic<bool> cachedExceptionStored_{false};
    std::exception_ptr cachedException_{};
  };
}

#endif /* art_Framework_Core_SharedException_h */

// Local Variables:
// mode: c++
// End:
