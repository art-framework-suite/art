#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/SummedValue.h"
#include <vector>

struct invalid_handle {};

int
main()
{
  art::Handle<std::vector<invalid_handle>> h;
  invalid_handle i;
  art::SummedValue<std::vector<invalid_handle>> s;
  s.update(i);
}
