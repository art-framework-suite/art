#include "art/Framework/Core/ProductRegistryHelper.h"

#include "art/Framework/Core/TypeLabelList.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Utilities/Exception.h"
#include "cpp0x/functional"
#include "cetlib/container_algorithms.h"

namespace {
  void
  addToRegistry(art::TypeLabel const &tl,
                art::ModuleDescription const& md,
                art::MasterProductRegistry& preg)
  {
    std::auto_ptr<art::BranchDescription>
      bdp(new art::BranchDescription
          (tl.branchType,
           tl.hasEmulatedModule() ? tl.emulatedModule : md.moduleLabel(),
           md.processName(),
           tl.className(),
           tl.friendlyClassName(),
           tl.productInstanceName,
           md));
    preg.addProduct(bdp);
  }
}

void
art::ProductRegistryHelper::
registerProducts(MasterProductRegistry& preg,
                 ModuleDescription const& md)
{
  cet::for_all(typeLabelList_,
               std::bind(&addToRegistry,
                         std::placeholders::_1,
                         std::cref(md),
                         std::ref(preg)));
}

void
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
}
