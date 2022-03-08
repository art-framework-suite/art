#include "art/Framework/Core/Modifier.h"
// vim: set sw=2 expandtab :

namespace art {
  Modifier::~Modifier() noexcept = default;
  Modifier::Modifier() : ProductRegistryHelper{product_creation_mode::produces}
  {}

  void
  Modifier::fillProductDescriptions()
  {
    ProductRegistryHelper::fillDescriptions(moduleDescription());
  }

  void
  Modifier::registerProducts(ProductDescriptions& productsToRegister)
  {
    ProductRegistryHelper::registerProducts(productsToRegister,
                                            moduleDescription());
  }
}
