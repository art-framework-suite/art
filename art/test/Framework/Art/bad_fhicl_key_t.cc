#include "art/Framework/Art/detail/fhicl_key.h"

#include <concepts>

struct inconvertible{};

int main() {
  inconvertible i;
  art::detail::fhicl_key<inconvertible>(i);
}
