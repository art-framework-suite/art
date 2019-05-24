#include "art/Framework/Core/ProductRegistryHelper.h"
// vim: set sw=2:

#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/detail/branchNameComponentChecking.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/container_algorithms.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include <cassert>
#include <memory>
#include <set>
#include <string>
#include <tuple>

using namespace std;

namespace art {

  ProductRegistryHelper::~ProductRegistryHelper() = default;
  ProductRegistryHelper::ProductRegistryHelper() = default;

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
      for (auto const& pr : expectedProducts) {
        productsToRegister.push_back(pr.second);
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
