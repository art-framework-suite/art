#include "art/Framework/EventProcessor/detail/ExceptionCollector.h"
#include "canvas/Utilities/Exception.h"

#include <cassert>
#include <memory>

using namespace art;

namespace {
  Exception make_exception_from_ptr(std::exception_ptr const& eptr)
    try {
      std::rethrow_exception(eptr);
    }
    catch (std::exception const& e) {
      return Exception{errors::OtherArt} << e.what();
    }
}

[[noreturn]]
void
art::detail::ExceptionCollector::rethrow()
{
  assert(!empty());

  auto top_e = std::make_unique<Exception>(make_exception_from_ptr(exceptions_.front()));
  for (auto i = std::begin(exceptions_)+1, e = std::end(exceptions_); i != e; ++i) {
    auto eptr = make_exception_from_ptr(*i);
    top_e->append(eptr);
  }
  exceptions_.clear();

  throw *top_e;
}
