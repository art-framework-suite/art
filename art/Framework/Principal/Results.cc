#include "art/Framework/Principal/Results.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "canvas/Persistency/Provenance/BranchType.h"

art::Results::Results(Principal const& p, ModuleDescription const& md) :
  DataViewImpl{p, md, InResults, false},
  principal_{p}
{
}

art::EDProductGetter const*
art::Results::productGetter(ProductID const pid) const
{
  return principal_.productGetter(pid);
}

void
art::Results::commit_(ResultsPrincipal& resp)
{
  for (auto& elem : putProducts()) {
    auto const& bd = elem.second.bd;
    auto productProvenancePtr = std::make_unique<ProductProvenance const>(bd.productID(),
                                                                          productstatus::present());
    resp.put(std::move(elem.second.prod),
             bd,
             std::move(productProvenancePtr));
  }

  // the cleanup is all or none
  putProducts().clear();
}
