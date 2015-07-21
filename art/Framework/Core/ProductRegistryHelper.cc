#include "art/Framework/Core/ProductRegistryHelper.h"
// vim: set sw=2:

#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/TypeLabel.h"
#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"

#include <cassert>
#include <string>

void
art::ProductRegistryHelper::
registerProducts(MasterProductRegistry& mpr,
                 ModuleDescription const& md)
{
  if (productList_) {
    BranchIDList bil;
    MasterProductRegistry::PerBranchTypePresence tp;
    for (auto const& val : *productList_) {
      auto const& bd = val.second;
      auto bid = bd.branchID().id();
      tp[bd.branchType()].emplace(bid);
      bil.push_back(bid);
    }
    FileBlock fb({}, "ProductRegistryHelper");
    mpr.initFromFirstPrimaryFile(*productList_, tp, fb);
    BranchIDListHelper::updateFromInput({bil}, fb.fileName());
    productList_.reset(); // Reset, since we no longer need it.
  }
  for (auto const& val : typeLabelList_) {
    mpr.addProduct(std::make_unique<art::BranchDescription>(val, md));
  }
}
