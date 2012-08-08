#include "art/Framework/Principal/RunPrincipal.h"

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

using art::RunPrincipal;

RunPrincipal::RunPrincipal(RunAuxiliary const& aux,
                           ProcessConfiguration const& pc,
                           std::unique_ptr<BranchMapper> && mapper,
                           std::unique_ptr<DelayedReader> && rtrv) :
  Principal(pc, aux.processHistoryID_, mapper, rtrv),
  aux_(aux) {
  if (ProductMetaData::instance().productProduced(InRun)) {
    addToProcessHistory();
  }
}

void
RunPrincipal::addOrReplaceGroup(std::unique_ptr<Group> && g) {
  cet::exempt_ptr<Group const> group =
    getExistingGroup(g->productDescription().branchID());
  if (!group) {
    addGroup_(g);
  } else {
    BranchDescription const& bd = group->productDescription();
    mf::LogWarning("RunMerging")
      << "Problem found while adding product provenance, "
      << "product already exists for ("
      << bd.friendlyClassName() << ","
      << bd.moduleLabel() << ","
      << bd.productInstanceName() << ","
      << bd.processName()
      << ")\n";
  }
}


void
RunPrincipal::addGroup(BranchDescription const& bd) {
  addOrReplaceGroup(gfactory::make_group(bd, ProductID()));
}

void
RunPrincipal::addGroup(std::unique_ptr<EDProduct> && prod,
                       BranchDescription const& bd) {
  addOrReplaceGroup(gfactory::make_group(prod, bd, ProductID()));
}

void
RunPrincipal::put(std::unique_ptr<EDProduct> && edp,
                  BranchDescription const& bd,
                  std::unique_ptr<ProductProvenance const> && productProvenance) {
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
  if (rp.get() == this) return; // Nothing to do.

  aux_.mergeAuxiliary(rp->aux());

  for (Principal::const_iterator i = rp->begin(), iEnd = rp->end(); i != iEnd; ++i) {

    std::auto_ptr<Group> g(new Group());
    g->swap(*i->second);

    addOrReplaceGroup(g);
  }
}
