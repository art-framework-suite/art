#include "art/Framework/Principal/Results.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Framework/Principal/ResultsPrincipal.h"

art::Results::Results(ResultsPrincipal& resp, ModuleDescription const& md) :
  DataViewImpl(resp, md, InResults)
{
}

void
art::Results::commit_() {
  // fill in guts of provenance here
  ResultsPrincipal & resp = dynamic_cast<ResultsPrincipal &>(principal());
  ProductPtrVec::iterator pit(putProducts().begin());
  ProductPtrVec::iterator pie(putProducts().end());

  while(pit!=pie) {
    std::unique_ptr<EDProduct> pr(pit->first);
    // note: ownership has been passed - so clear the pointer!
    pit->first = 0;

    // set provenance
    std::unique_ptr<ProductProvenance const> subRunProductProvenancePtr(
      new ProductProvenance(pit->second->branchID(),
                            productstatus::present()));
    resp.put(std::move(pr),
             *pit->second,
             std::move(subRunProductProvenancePtr));
    ++pit;
  }

  // the cleanup is all or none
  putProducts().clear();
}
