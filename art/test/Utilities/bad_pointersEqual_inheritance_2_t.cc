#include "art/Utilities/pointersEqual.h"

namespace {
  class A {};
  class B {};
}

int main()
{
  A a1;
  B b1;
  (void) art::pointersEqual(&a1, &b1);
}
