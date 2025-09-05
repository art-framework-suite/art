#include "art/Utilities/pointersEqual.h"

namespace {
  class A {};
  class B {};
  class C : public A, public B {};
  struct D : public C {};
}

int main()
{
  D d1;
  A* pd1a1(&d1);
  B* pd1b1(&d1);
  (void) art::pointersEqual(pd1a1, pd1b1);
}
