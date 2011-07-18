#include "art/Framework/Core/SubRun.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Core/Run.h"

namespace art {

  namespace {
    Run *
    newRun(SubRunPrincipal& srp, ModuleDescription const& md) {
      return (srp.runPrincipalSharedPtr() ? new Run(srp.runPrincipal(), md) : 0);
    }
  }

  SubRun::SubRun(SubRunPrincipal& srp, ModuleDescription const& md) :
	DataViewImpl(srp, md, InSubRun),
	aux_(srp.aux()),
	run_(newRun(srp, md)) {
  }

  SubRunPrincipal &
  SubRun::subRunPrincipal() {
    return dynamic_cast<SubRunPrincipal &>(principal());
  }

  SubRunPrincipal const &
  SubRun::subRunPrincipal() const {
    return dynamic_cast<SubRunPrincipal const&>(principal());
  }

  Run const&
  SubRun::getRun() const {
    if (!run_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL run.\n";
    }
    return *run_;
  }

  void
  SubRun::commit_() {
    // fill in guts of provenance here
    SubRunPrincipal & srp = subRunPrincipal();
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
	srp.put(pr, *pit->second, subRunEntryInfoPtr);
	++pit;
    }

    // the cleanup is all or none
    putProducts().clear();
  }

}
