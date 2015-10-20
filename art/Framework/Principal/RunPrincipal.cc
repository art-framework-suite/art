#include "art/Framework/Principal/RunPrincipal.h"
// vim: set sw=2:

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace art {

RunPrincipal::
RunPrincipal(RunAuxiliary const& aux,
             ProcessConfiguration const& pc,
             std::unique_ptr<BranchMapper>&& mapper,
             std::unique_ptr<DelayedReader>&& rtrv, int idx,
             RunPrincipal* primaryPrincipal)
  : Principal(pc, aux.processHistoryID_, std::move(mapper), std::move(rtrv),
              idx, primaryPrincipal)
  , aux_(aux)
{
  if (ProductMetaData::instance().productProduced(InRun)) {
    addToProcessHistory();
  }
}

BranchType
RunPrincipal::
branchType() const
{
  return InRun;
}

ProcessHistoryID const&
RunPrincipal::
processHistoryID() const
{
  return aux().processHistoryID_;
}

void
RunPrincipal::
setProcessHistoryID(ProcessHistoryID const& phid)
{
  return aux().setProcessHistoryID(phid);
}

void
RunPrincipal::
addOrReplaceGroup(std::unique_ptr<Group>&& g)
{
  cet::exempt_ptr<Group const> group =
    getExistingGroup(g->productDescription().branchID());
  if (!group) {
    addGroup_(std::move(g));
    return;
  }
  BranchDescription const& bd = group->productDescription();
  mf::LogWarning("RunMerging")
      << "Problem found while adding product provenance, "
      << "product already exists for ("
      << bd.friendlyClassName()
      << ","
      << bd.moduleLabel()
      << ","
      << bd.productInstanceName()
      << ","
      << bd.processName()
      << ")\n";
}

void
RunPrincipal::
addGroup(BranchDescription const& bd)
{
  addOrReplaceGroup(gfactory::make_group(bd, ProductID()));
}

void
RunPrincipal::
addGroup(std::unique_ptr<EDProduct>&& prod, BranchDescription const& bd)
{
  addOrReplaceGroup(gfactory::make_group(std::move(prod), bd, ProductID()));
}

void
RunPrincipal::
mergeRun(std::shared_ptr<RunPrincipal> rp)
{
  if (rp.get() == this) {
    // Nothing to do.
    return;
  }
  aux_.mergeAuxiliary(rp->aux());
  for (auto I = rp->cbegin(), E = rp->cend(); I != E; ++I) {
    std::unique_ptr<Group> g(new Group());
    g->swap(*I->second);
    addOrReplaceGroup(std::move(g));
  }
}

void
RunPrincipal::
put(std::unique_ptr<EDProduct>&& edp, BranchDescription const& bd,
    std::unique_ptr<ProductProvenance const>&& productProvenance)
{
  if (!edp) {
    throw Exception(errors::InsertFailure, "Null Pointer")
        << "put: Cannot put because unique_ptr to product is null."
        << "\n";
  }
  branchMapper().insert(std::move(productProvenance));
  this->addGroup(std::move(edp), bd);
}

} // namespace art

