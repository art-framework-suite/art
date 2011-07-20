#include "art/Framework/Core/detail/MasterProductRegistry.h"

#include <ostream>

std::ostream &
operator<<(std::ostream &os, art::MasterProductRegistry const &mpr) {
  mpr.print(os);
  return os;
}

void
art::MasterProductRegistry::print(std::ostream &os) const {
  // FIXME: Implement.
}
