#include "art/Utilities/pointersEqual.h"

#include <utility>

int
main()
{
  int i = 3;
  double p = i;
  std::ignore = art::pointersEqual(&i, &p);
}
