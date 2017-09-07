#include <cassert>
#include <iostream>
#include <stdexcept>

#include "cetlib/exception.h"
#include "art/Utilities/CPCSentry.h"
#include "art/Utilities/CurrentProcessingContext.h"

int work()
{
  //art::CurrentProcessingContext ctx;
  //cet::exempt_ptr<art::CurrentProcessingContext const> ptr {nullptr};
  //assert(ptr.get() == nullptr);
  //{
  //  art::detail::CPCSentry sentry{ptr, &ctx};
  //  assert(ptr.get() == &ctx);
  //}
  //assert(ptr.get() == nullptr);
  return 0;
}


int main()
{
  //int rc = -1;
  //try { rc = work(); }
  //catch (cet::exception& x) {
  //    std::cerr << "cet::exception caught\n";
  //    std::cerr << x.what() << '\n';
  //    rc = -2;
  //}
  //catch (std::exception& x) {
  //    std::cerr << "std::exception caught\n";
  //    std::cerr << x.what() << '\n';
  //    rc = -3;
  //}
  //catch (...) {
  //    std::cerr << "Unknown exception caught\n";
  //    rc = -4;
  //}
  //return rc;
}
