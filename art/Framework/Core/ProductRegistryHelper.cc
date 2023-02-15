#include "art/Framework/Core/ProductRegistryHelper.h"
// vim: set sw=2:

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "cetlib/container_algorithms.h"
#include "cetlib_except/exception.h"
#include "range/v3/view.hpp"

#include <memory>

using namespace std;

namespace art {

  ProductRegistryHelper::~ProductRegistryHelper() = default;

  ProductRegistryHelper::ProductRegistryHelper(product_creation_mode const mode)
    : mode_{mode}
  {}

  void
  ProductRegistryHelper::registerProducts(
    ProductDescriptions& productsToRegister,
    ModuleDescription const& md)
  {
    // Possible products from input source
    if (productList_) {
      cet::transform_all(*productList_,
                         back_inserter(productsToRegister),
                         [](auto const& pr) { return pr.second; });
    }

    // Now fill the descriptions for products that are to be produced.
    fillDescriptions(md);
    auto registerProductsPerBT = [this,
                                  &productsToRegister](BranchType const bt) {
      auto const& expectedProducts = collector_.expectedProducts(bt);
      for (auto const& pd : expectedProducts | ::ranges::views::values) {
        productsToRegister.push_back(pd);
      }
    };
    for_each_branch_type(registerProductsPerBT);
  }

  void
  ProductRegistryHelper::fillDescriptions(ModuleDescription const& md)
  {
    collector_.fillDescriptions(md);
  }

} // namespace art
