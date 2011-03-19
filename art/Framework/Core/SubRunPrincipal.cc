#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/Group.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/ProductID.h"

namespace art {

  SubRunPrincipal::SubRunPrincipal(SubRunAuxiliary const& aux,
                                   cet::exempt_ptr<ProductRegistry const> reg,
				   ProcessConfiguration const& pc,
				   boost::shared_ptr<BranchMapper> mapper,
				   boost::shared_ptr<DelayedReader> rtrv) :
    Base(reg, pc, aux.processHistoryID_, mapper, rtrv),
    runPrincipal_(),
    aux_(aux) 
  {
    if (reg->productProduced(InSubRun)) {
      addToProcessHistory();
    }
  }

  void
  SubRunPrincipal::addOrReplaceGroup(std::auto_ptr<Group> g) {

    Group* group = getExistingGroup(*g);
    if (group != 0) {

      if (!group->productUnavailable()) {
        assert(group->product() != 0);
      }
      if (!g->productUnavailable()) {
        assert(g->product() != 0);
      }

      group->mergeGroup(g.get());
    } else {
      addGroup_(g);
    }
  }

  void
  SubRunPrincipal::addGroup(ConstBranchDescription const& bd) {
    std::auto_ptr<Group> g(new Group(bd, ProductID()));
    addOrReplaceGroup(g);
  }

  void
  SubRunPrincipal::addGroup(std::auto_ptr<EDProduct> prod,
			    ConstBranchDescription const& bd,
			    std::auto_ptr<ProductProvenance> productProvenance) {
    std::auto_ptr<Group> g(new Group(prod, bd, ProductID(), productProvenance));
    addOrReplaceGroup(g);
  }

  void
  SubRunPrincipal::addGroup(ConstBranchDescription const& bd,
			    std::auto_ptr<ProductProvenance> productProvenance) {
    std::auto_ptr<Group> g(new Group(bd, ProductID(), productProvenance));
    addOrReplaceGroup(g);
  }

  void
  SubRunPrincipal::put(std::auto_ptr<EDProduct> edp,
		       ConstBranchDescription const& bd,
		       std::auto_ptr<ProductProvenance> productProvenance) {

    if (edp.get() == 0) {
      throw art::Exception(art::errors::InsertFailure,"Null Pointer")
	<< "put: Cannot put because auto_ptr to product is null."
	<< "\n";
    }
    branchMapperPtr()->insert(*productProvenance);
    // Group assumes ownership
    this->addGroup(edp, bd, productProvenance);
  }

  void
  SubRunPrincipal::mergeSubRun(boost::shared_ptr<SubRunPrincipal> srp) {

    aux_.mergeAuxiliary(srp->aux());

    for (Base::const_iterator i = srp->begin(), iEnd = srp->end(); i != iEnd; ++i) {

      std::auto_ptr<Group> g(new Group());
      g->swap(*i->second);

      addOrReplaceGroup(g);
    }
  }
}

