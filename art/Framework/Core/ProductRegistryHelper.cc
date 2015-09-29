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

#include <string>

namespace {
  using PerBranchTypePresence = art::MasterProductRegistry::PerBranchTypePresence;
}

void
art::ProductRegistryHelper::
registerProducts(MasterProductRegistry& mpr,
                 ModuleDescription const& md)
{
  if (productList_) {
    FileBlock fb({}, "ProductRegistryHelper");
    mpr.initFromFirstPrimaryFile(*productList_.get(),
                                 perBranchTypePresence_(md),
                                 fb);
    BranchIDList bil;
    for (auto const& val : *productList_.get()) {
        bil.push_back(val.second.branchID().id());
    }
    BranchIDListHelper::updateFromInput({bil}, fb.fileName());
    productList_.reset(); // No longer needed.
  }
  for (auto const& val : typeLabelList_) {
    mpr.addProduct(std::make_unique<art::BranchDescription>(val, md));
  }
}

PerBranchTypePresence
art::ProductRegistryHelper::
perBranchTypePresence_(ModuleDescription const &md)
{
  PerBranchTypePresence result;
  for (auto const& tl : typeLabelList_) {
    std::string branchName;
    branchName += tl.friendlyClassName();
    branchName += "_";
    branchName += md.moduleLabel();
    branchName += "_";
    branchName += tl.productInstanceName;
    branchName += "_";
    branchName += md.processName();
    branchName += ".";
    result[tl.branchType].emplace( BranchID( branchName ) );
  }
  return result;
}
