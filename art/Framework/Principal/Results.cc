#include "art/Framework/Principal/Results.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "cetlib/container_algorithms.h"

#include <vector>

art::Results::Results(ResultsPrincipal const& resp, ModuleDescription const& md) :
  DataViewImpl{resp, md, InResults, false}
{
}

void
art::Results::commit_(ResultsPrincipal& resp)
{
  std::vector<BranchID> gotBranchIDVector;
  auto const& gotBranchIDs = retrievedProducts();
  if (!gotBranchIDs.empty()) {
    gotBranchIDVector.reserve(gotBranchIDs.size());
    gotBranchIDVector.assign(gotBranchIDs.begin(), gotBranchIDs.end());
  }

  auto put_in_principal = [&resp, &gotBranchIDVector](auto& elem) {
    auto const& bd = elem.second.bd;
    auto resultsProductProvenancePtr = std::make_unique<ProductProvenance const>(bd.branchID(),
                                                                                 productstatus::present(),
                                                                                 gotBranchIDVector);
    resp.put(std::move(elem.second.prod),
             bd,
             std::move(resultsProductProvenancePtr));
  };
  cet::for_all(putProducts(), put_in_principal);

  // the cleanup is all or none
  putProducts().clear();
}
