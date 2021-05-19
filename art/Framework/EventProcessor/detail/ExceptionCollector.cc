#include "art/Framework/EventProcessor/detail/ExceptionCollector.h"

#include <cassert>

using namespace art;

namespace {
  std::string
  exception_msg_from_ptr(std::exception_ptr const eptr) try {
    std::rethrow_exception(eptr);
  }
  catch (std::exception const& e) {
    return e.what();
  }
} // namespace

[[noreturn]] void
art::detail::ExceptionCollector::rethrow()
{
  assert(!empty());
  std::string message;
  for (auto const e : exceptions_) {
    message += exception_msg_from_ptr(e);
  }
  exceptions_.clear();
  throw collected_exception{move(message)};
}
