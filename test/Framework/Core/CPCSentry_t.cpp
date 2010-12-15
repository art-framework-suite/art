#include <cassert>
#include <iostream>
#include <stdexcept>

#include "cetlib/exception.h"
#include "art/Framework/Core/CurrentProcessingContext.h"
#include "art/Framework/Core/CPCSentry.h"

int work()
{
  art::CurrentProcessingContext ctx;
  art::CurrentProcessingContext const* ptr = 0;
  assert(ptr == 0);
  {
    art::detail::CPCSentry sentry(ptr, &ctx);
    assert(ptr == &ctx);
  }
  assert(ptr == 0);
  return 0;
}


int main()
{
  int rc = -1;
  try { rc = work(); }
  catch (cet::exception& x) {
      std::cerr << "cet::exception caught\n";
      std::cerr << x.what() << '\n';
      rc = -2;
  }
  catch (std::exception& x) {
      std::cerr << "std::exception caught\n";
      std::cerr << x.what() << '\n';
      rc = -3;
  }
  catch (...) {
      std::cerr << "Unknown exception caught\n";
      rc = -4;
  }
  return rc;
}
