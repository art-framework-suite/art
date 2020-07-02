#include "art/Framework/Core/Modifier.h"
// vim: set sw=2 expandtab :

namespace art {
  Modifier::~Modifier() noexcept = default;
  Modifier::Modifier() : ProductRegistryHelper{product_creation_mode::produces}
  {}
}
