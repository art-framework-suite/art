#include "art/Framework/Principal/Results.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "canvas/Persistency/Provenance/BranchType.h"

art::Results::Results(Principal const& p,
                      ModuleDescription const& md,
                      cet::exempt_ptr<Consumer> consumer)
  : DataViewImpl{p, md, InResults, false, consumer}, principal_{p}
{}

art::EDProductGetter const*
art::Results::productGetter(ProductID const pid) const
{
  return principal_.productGetter(pid);
}

void
art::Results::commit(ResultsPrincipal& resp)
{
  for (auto& elem : putProducts()) {
    auto const& pd = elem.second.pd;
    auto productProvenancePtr = std::make_unique<ProductProvenance const>(
      pd.productID(), productstatus::present());
    resp.put(std::move(elem.second.prod), pd, std::move(productProvenancePtr));
  }

  // the cleanup is all or none
  putProducts().clear();
}
