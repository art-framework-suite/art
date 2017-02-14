#include "art/Framework/Core/ProductRegistryHelper.h"
// vim: set sw=2:

#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchIDList.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include <cassert>
#include <string>

void
art::ProductRegistryHelper::
registerProducts(MasterProductRegistry& mpr,
                 ModuleDescription const& md)
{
  if (productList_) {
    BranchIDList bil;
    PerBranchTypePresence tp;
    for (auto const& val : *productList_) {
      auto const& bd = val.second;
      auto bid = bd.branchID().id();
      tp[bd.branchType()].emplace(bid);
      bil.push_back(bid);
    }
    FileBlock fb({}, "ProductRegistryHelper");
    mpr.initFromFirstPrimaryFile(*productList_, tp, fb);
    BranchIDListRegistry::updateFromInput({bil}, fb.fileName());
    productList_.reset(); // Reset, since we no longer need it.
  }
  for (auto const& val : typeLabelList_) {
    auto bd = std::make_unique<art::BranchDescription>(val, md);
    auto & expProducts = expectedProducts_[bd->branchType()];
    std::ostringstream oss;
    oss << *bd;
    expProducts.emplace(bd->branchID(), oss.str());
    mpr.addProduct(std::move(bd));
  }
}
