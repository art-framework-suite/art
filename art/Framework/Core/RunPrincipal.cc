#include "art/Framework/Core/RunPrincipal.h"

#include "art/Framework/Core/Group.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductRegistry.h"

namespace art {
  RunPrincipal::RunPrincipal(RunAuxiliary const& aux,
                             cet::exempt_ptr<ProductRegistry const> reg,
			     ProcessConfiguration const& pc,
			     std::shared_ptr<BranchMapper> mapper,
			     std::shared_ptr<DelayedReader> rtrv) :
    Base(reg, pc, aux.processHistoryID_, mapper, rtrv),
    aux_(aux) {
    if (reg->productProduced(InRun)) {
      addToProcessHistory();
    }
  }

  void
  RunPrincipal::addOrReplaceGroup(std::auto_ptr<Group> g) {

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
  RunPrincipal::addGroup(ConstBranchDescription const& bd) {
    std::auto_ptr<Group> g(new Group(bd, ProductID()));
    addOrReplaceGroup(g);
  }

  void
  RunPrincipal::addGroup(std::auto_ptr<EDProduct> prod,
			 ConstBranchDescription const& bd,
			 std::auto_ptr<ProductProvenance> productProvenance) {
    std::auto_ptr<Group> g(new Group(prod, bd, ProductID(), productProvenance));
    addOrReplaceGroup(g);
  }

  void
  RunPrincipal::addGroup(ConstBranchDescription const& bd,
			 std::auto_ptr<ProductProvenance> productProvenance) {
    std::auto_ptr<Group> g(new Group(bd, ProductID(), productProvenance));
    addOrReplaceGroup(g);
  }

  void
  RunPrincipal::put(std::auto_ptr<EDProduct> edp,
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
  RunPrincipal::mergeRun(std::shared_ptr<RunPrincipal> rp) {

    aux_.mergeAuxiliary(rp->aux());

    for (Base::const_iterator i = rp->begin(), iEnd = rp->end(); i != iEnd; ++i) {

      std::auto_ptr<Group> g(new Group());
      g->swap(*i->second);

      addOrReplaceGroup(g);
    }
  }
}  // art
