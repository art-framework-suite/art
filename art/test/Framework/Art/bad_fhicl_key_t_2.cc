#include "art/Framework/Art/detail/fhicl_key.h"

#include <concepts>

struct inconvertible {};

int
main()
{
  inconvertible i;
  inconvertible i_2;
  std::string name{"name"};
  art::detail::fhicl_key<std::string, inconvertible, inconvertible>(
    name, i, i_2);
}
