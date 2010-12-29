/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Framework/Core/ProductRegistryHelper.h"

#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ProductRegistry.h"

namespace art {
  ProductRegistryHelper::~ProductRegistryHelper() { }

  ProductRegistryHelper::TypeLabelList & ProductRegistryHelper::typeLabelList() {
    return typeLabelList_;
  }

  void
  ProductRegistryHelper::addToRegistry(TypeLabelList::const_iterator const& iBegin,
                                       TypeLabelList::const_iterator const& iEnd,
                                       ModuleDescription const& iDesc,
                                       ProductRegistry& iReg,
                                       bool iIsListener) {
    for (TypeLabelList::const_iterator p = iBegin; p != iEnd; ++p) {
      BranchDescription pdesc(p->branchType_,
                              iDesc.moduleLabel(),
                              iDesc.processName(),
                              p->typeID_.userClassName(),
                              p->typeID_.friendlyClassName(),
                              p->productInstanceName_,
                              iDesc);
      if (!p->branchAlias_.empty()) pdesc.branchAliases().insert(p->branchAlias_);
      iReg.addProduct(pdesc, iIsListener);
    }//for
  }

}  // art
