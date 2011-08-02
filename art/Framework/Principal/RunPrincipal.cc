#include "art/Framework/Principal/RunPrincipal.h"

#include "art/Persistency/Common/Group.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Persistency/Provenance/ProductID.h"

using art::RunPrincipal;

RunPrincipal::RunPrincipal(RunAuxiliary const& aux,
                           ProcessConfiguration const& pc,
                           std::auto_ptr<BranchMapper> mapper,
                           std::auto_ptr<DelayedReader> rtrv) :
  Principal(pc, aux.processHistoryID_, mapper, rtrv),
  aux_(aux) {
  if (ProductMetaData::instance().productProduced(InRun)) {
    addToProcessHistory();
  }
}

void
RunPrincipal::addOrReplaceGroup(std::auto_ptr<Group> g) {
  Group* group = getExistingGroup(*g);
  if (group == 0) {
    addGroup_(g);
  } else {
    assert(group->productUnavailable() || group->product());
    assert(g->productUnavailable() || g->product());
    group->mergeGroup(g.get());
  }
}


void
RunPrincipal::addGroup(BranchDescription const& bd) {
  std::auto_ptr<Group> g(new Group(bd, ProductID()));
  addOrReplaceGroup(g);
}

void
RunPrincipal::addGroup(std::auto_ptr<EDProduct> prod,
                       BranchDescription const& bd) {
  std::auto_ptr<Group> g(new Group(prod, bd, ProductID()));
  addOrReplaceGroup(g);
}

void
RunPrincipal::put(std::auto_ptr<EDProduct> edp,
                  BranchDescription const& bd,
                  std::auto_ptr<ProductProvenance const> productProvenance) {
  if (edp.get() == 0) {
    throw art::Exception(art::errors::InsertFailure,"Null Pointer")
      << "put: Cannot put because auto_ptr to product is null."
      << "\n";
  }
  branchMapper().insert(productProvenance);
  this->addGroup(edp, bd);
}

void
RunPrincipal::mergeRun(std::shared_ptr<RunPrincipal> rp) {

  aux_.mergeAuxiliary(rp->aux());

  for (Principal::const_iterator i = rp->begin(), iEnd = rp->end(); i != iEnd; ++i) {

    std::auto_ptr<Group> g(new Group());
    g->swap(*i->second);

    addOrReplaceGroup(g);
  }
}
