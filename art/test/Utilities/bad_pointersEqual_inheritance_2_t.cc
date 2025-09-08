#include "art/Utilities/pointersEqual.h"

#include <utility>

namespace {
  class A {};
  class B {};
}

int
main()
{
  A a1;
  B b1;
  std::ignore = art::pointersEqual(&a1, &b1);
}
