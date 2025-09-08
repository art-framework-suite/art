#include "art/Utilities/pointersEqual.h"

#include <utility>

namespace {
  class A {};
  class B {};
  class C : public A, public B {};
  struct D : public C {};
}

int
main()
{
  D d1;
  D* pd1d1(&d1);
  int* ip = new int;
  std::ignore = art::pointersEqual(pd1d1, ip);
  delete ip;
}
