#include "art/Framework/Core/ProductRegistryHelper.h"

#include "art/Framework/Core/TypeLabelList.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Framework/Core/MasterProductRegistry.h"

namespace art
{
  TypeLabelList&
  ProductRegistryHelper::typeLabelList()
  {
    return typeLabelList_;
  }

  void
  ProductRegistryHelper::addToRegistry(TypeLabelList::iterator i,
                                       TypeLabelList::iterator e,
                                       ModuleDescription const& md,
                                       MasterProductRegistry& preg,
                                       bool isListener)
  {
    assert(!isListener);
    for ( ;  i != e; ++i)
      {
        std::auto_ptr<BranchDescription>
          bdp(new BranchDescription(i->branchType,
                                    i->hasEmulatedModule() ? i->emulatedModule : md.moduleLabel(),
                                    md.processName(),
                                    i->userClassName(),
                                    i->friendlyClassName(),
                                    i->productInstanceName,
                                    md));
        if (i->hasBranchAlias())
          bdp->addBranchAlias(i->branchAlias);

        preg.addProduct(bdp);
      }
  }

}  // art
