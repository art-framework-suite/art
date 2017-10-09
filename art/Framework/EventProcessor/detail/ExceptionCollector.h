#ifndef art_Framework_EventProcessor_detail_ExceptionCollector_h
#define art_Framework_EventProcessor_detail_ExceptionCollector_h

// ======================================================================
//
// ExceptionCollector - Utility used to collect exceptions thrown when
// invoking functions that were provided to ExceptionCollector::call.
// The semantics are different than cet::exception_collector in that
// the (compiled-generated) ExceptionCollector d'tor DOES NOT call
// rethrow.
//
// TODO: determine if this class should be removed in favor of the
// cet::exception_collector class.
//
// ======================================================================

#include <exception>
#include <vector>

namespace art {
  namespace detail {
    class ExceptionCollector;
  }
} // namespace art

class art::detail::ExceptionCollector {
public:
  bool
  empty() const noexcept
  {
    return exceptions_.empty();
  }
  [[noreturn]] void rethrow();

  template <typename F>
  void
  call(F f) try {
    f();
  }
  catch (...) {
    exceptions_.push_back(std::current_exception());
  }

private:
  std::vector<std::exception_ptr> exceptions_{};
};

#endif /* art_Framework_EventProcessor_detail_ExceptionCollector_h */

// Local Variables:
// mode: c++
// End:
