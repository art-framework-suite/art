#include "art/Framework/Core/ProductRegistryHelper.h"

#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/TypeLabel.h"
#include "art/Utilities/DebugMacros.h"
#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"

#include <string>

namespace {
  void
  addToRegistry(art::TypeLabel const &tl,
                art::ModuleDescription const& md,
                art::MasterProductRegistry& preg)
  {
    preg.addProduct(std::unique_ptr<art::BranchDescription>
                    (new art::BranchDescription(tl, md)));
  }
}

void
art::ProductRegistryHelper::
registerProducts(MasterProductRegistry& preg,
                 ModuleDescription const& md)
{
  if (productList_) {
    FDEBUG(1) << "ProductRegistryHelper::registerProducts: "
              "merging product list ...\n";
    preg.updateFromInput(*productList_.get());
    FDEBUG(1) << "ProductRegistryHelper::registerProducts: "
              "product list merged.\n";
    BranchIDList bil;
    for (auto I = productList_->begin(), E = productList_->end();
            I != E; ++I) {
        bil.push_back(I->second.branchID().id());
    }
    const std::string meAsAFilename("ProductRegistryHelper");
    BranchIDLists bidlists;
    bidlists.push_back(bil);
    BranchIDListHelper::updateFromInput(bidlists, meAsAFilename);
    if (art::debugit() >= 2) {
      for (art::ProductList::const_iterator I = preg.productList().begin(),
           E = preg.productList().end(); I != E; ++I) {
        FDEBUG(2) << "ProductRegistryHelper::registerProducts: "
                  << I->first << '\n';
        FDEBUG(2) << "ProductRegistryHelper::registerProducts: present: "
                  << I->second.present() << '\n';
        FDEBUG(2) << "ProductRegistryHelper::registerProducts: produced: "
                  << I->second.produced() << '\n';
      }
    }
    productList_.reset();
  }
  cet::for_all(typeLabelList_,
	       [&, this](auto const& tl){ addToRegistry(tl, md, preg); });
}

art::TypeLabel const &
art::ProductRegistryHelper::
insertOrThrow(TypeLabel const &tl) {
  std::pair<TypeLabelList::iterator, bool>
    result(typeLabelList_.insert(tl));
  if (!result.second) {
    throw Exception(errors::LogicError, "RegistrationFailure")
      << "The module being constructed attempted to register conflicting products with:\n"
      << "friendlyClassName: "
      << tl.friendlyClassName()
      << " and instanceName: "
      << tl.productInstanceName
      << ".\n";
  }
  return *result.first;
}
