#include "art/Framework/Principal/Results.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "canvas/Persistency/Provenance/BranchType.h"

art::Results::Results(ResultsPrincipal const& resp, ModuleDescription const& md) :
  DataViewImpl{resp, md, InResults, false}
{
}

void
art::Results::commit_(ResultsPrincipal& resp)
{
  for (auto& elem : putProducts()) {
    auto const& bd = elem.second.bd;
    auto productProvenancePtr = std::make_unique<ProductProvenance const>(BranchID{bd.productID().value()},
                                                                          productstatus::present());
    resp.put(std::move(elem.second.prod),
             bd,
             std::move(productProvenancePtr));
  }

  // the cleanup is all or none
  putProducts().clear();
}
