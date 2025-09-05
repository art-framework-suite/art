#include "art/Utilities/pointersEqual.h"

int main()
{
  int i = 3;
  double p = i;
  (void) art::pointersEqual(&i, &p);
}
