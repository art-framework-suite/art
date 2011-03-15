/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/TypeLabelList.h"

#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ProductRegistry.h"

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
                                       ProductRegistry& preg,
                                       bool isListener)
  {

    for ( ;  i != e; ++i)
      {
        BranchDescription pdesc(i->branchType,
                                md.moduleLabel(),
                                md.processName(),
                                i->userClassName(),
                                i->friendlyClassName(),
                                i->productInstanceName,
                                md);
        if (i->hasBranchAlias())
          {
            pdesc.addBranchAlias(i->branchAlias);
          }

        preg.addProduct(pdesc, isListener);
      }
  }

}  // art
