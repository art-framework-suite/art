#include "art/Framework/Core/ProductRegistryHelper.h"

#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/TypeLabel.h"
#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cpp0x/functional"

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
