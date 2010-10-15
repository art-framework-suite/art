#include "art/Framework/Core/SubRun.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Core/Run.h"

namespace art {

  namespace {
    Run *
    newRun(SubRunPrincipal& lbp, ModuleDescription const& md) {
      return (lbp.runPrincipalSharedPtr() ? new Run(lbp.runPrincipal(), md) : 0);
    }
  }

  SubRun::SubRun(SubRunPrincipal& lbp, ModuleDescription const& md) :
	DataViewImpl(lbp, md, InSubRun),
	aux_(lbp.aux()),
	run_(newRun(lbp, md)) {
  }

  SubRunPrincipal &
  SubRun::subRunPrincipal() {
    return dynamic_cast<SubRunPrincipal &>(principal());
  }

  SubRunPrincipal const &
  SubRun::subRunPrincipal() const {
    return dynamic_cast<SubRunPrincipal const&>(principal());
  }

  Provenance
  SubRun::getProvenance(BranchID const& bid) const
  {
    return subRunPrincipal().getProvenance(bid);
  }

  void
  SubRun::getAllProvenance(std::vector<Provenance const*> & provenances) const
  {
    subRunPrincipal().getAllProvenance(provenances);
  }


  void
  SubRun::commit_() {
    // fill in guts of provenance here
    SubRunPrincipal & lbp = subRunPrincipal();
    ProductPtrVec::iterator pit(putProducts().begin());
    ProductPtrVec::iterator pie(putProducts().end());

    while(pit!=pie) {
	std::auto_ptr<EDProduct> pr(pit->first);
	// note: ownership has been passed - so clear the pointer!
	pit->first = 0;

	// set provenance
	std::auto_ptr<ProductProvenance> subRunEntryInfoPtr(
		new ProductProvenance(pit->second->branchID(),
				    productstatus::present()));
	lbp.put(pr, *pit->second, subRunEntryInfoPtr);
	++pit;
    }

    // the cleanup is all or none
    putProducts().clear();
  }

}
