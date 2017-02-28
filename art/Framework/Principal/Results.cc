#include "art/Framework/Principal/Results.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "cetlib/container_algorithms.h"

art::Results::Results(ResultsPrincipal const& resp, ModuleDescription const& md) :
  DataViewImpl{resp, md, InResults}
{
}

void
art::Results::commit_(ResultsPrincipal& resp) {
  auto put_in_principal = [&resp](auto& elem) {
    auto resultsProductProvenancePtr = std::make_unique<ProductProvenance const>(elem.first,
                                                                                 productstatus::present());
    resp.put(std::move(elem.second.prod),
             elem.second.bd,
             std::move(resultsProductProvenancePtr));
  };
  cet::for_all(putProducts(), put_in_principal);

  // the cleanup is all or none
  putProducts().clear();
}
