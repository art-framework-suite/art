#include "art/Framework/Principal/SubRunPrincipal.h"
// vim: set sw=2:

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace art {

SubRunPrincipal::
SubRunPrincipal(SubRunAuxiliary const& aux, ProcessConfiguration const& pc,
                std::unique_ptr<BranchMapper>&& mapper,
                std::unique_ptr<DelayedReader>&& rtrv, int idx,
                SubRunPrincipal* primaryPrincipal)
  : Principal(pc, aux.processHistoryID_, std::move(mapper), std::move(rtrv),
              idx, primaryPrincipal)
  , runPrincipal_()
  , aux_(aux)
{
  if (ProductMetaData::instance().productProduced(InSubRun)) {
    addToProcessHistory();
  }
}

BranchType
SubRunPrincipal::
branchType() const
{
  return InSubRun;
}

ProcessHistoryID const&
SubRunPrincipal::
processHistoryID() const
{
  return aux().processHistoryID_;
}

void
SubRunPrincipal::
setProcessHistoryID(ProcessHistoryID const& phid)
{
  return aux().setProcessHistoryID(phid);
}

void
SubRunPrincipal::
addOrReplaceGroup(std::unique_ptr<Group>&& g)
{
  cet::exempt_ptr<Group const> group =  getExistingGroup(g->productDescription().branchID());
  if (!group) {
    addGroup_(std::move(g));
  }
}

void
SubRunPrincipal::
addGroup(BranchDescription const& bd)
{
  addOrReplaceGroup(gfactory::make_group(bd, ProductID()));
}

void
SubRunPrincipal::
addGroup(std::unique_ptr<EDProduct>&& prod, BranchDescription const& bd)
{
  addOrReplaceGroup(gfactory::make_group(std::move(prod), bd, ProductID()));
}

void
SubRunPrincipal::
put(std::unique_ptr<EDProduct>&& edp, BranchDescription const& bd,
    std::unique_ptr<ProductProvenance const>&& productProvenance)
{
  if (!edp) {
    throw art::Exception(art::errors::InsertFailure, "Null Pointer")
        << "put: Cannot put because unique_ptr to product is null."
        << "\n";
  }
  branchMapper().insert(std::move(productProvenance));
  this->addGroup(std::move(edp), bd);
}

RunPrincipal const&
SubRunPrincipal::
runPrincipal() const
{
  if (!runPrincipal_) {
    throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL runPrincipal.\n";
  }
  return *runPrincipal_;
}

RunPrincipal&
SubRunPrincipal::
runPrincipal()
{
  if (!runPrincipal_) {
    throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL runPrincipal.\n";
  }
  return *runPrincipal_;
}

} // namespace art
