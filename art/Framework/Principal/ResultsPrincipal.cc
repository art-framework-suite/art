#include "art/Framework/Principal/ResultsPrincipal.h"
// vim: set sw=2:

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

art::ResultsPrincipal::
ResultsPrincipal(ResultsAuxiliary const & aux,
                 ProcessConfiguration const & pc,
                 std::unique_ptr<BranchMapper> && mapper,
                 std::unique_ptr<DelayedReader> && rtrv, int idx,
                 ResultsPrincipal * primaryPrincipal)
  : Principal(pc, aux.processHistoryID_, std::move(mapper), std::move(rtrv),
              idx, primaryPrincipal),
    aux_(aux)
{
}

art::BranchType
art::ResultsPrincipal::
branchType() const
{
  return InResults;
}

art::ProcessHistoryID const&
art::ResultsPrincipal::
processHistoryID() const
{
  return aux().processHistoryID_;
}

void
art::ResultsPrincipal::
setProcessHistoryID(ProcessHistoryID const& phid)
{
  aux().setProcessHistoryID(phid);
}

void
art::ResultsPrincipal::
addOrReplaceGroup(std::unique_ptr<Group>&& g)
{
  cet::exempt_ptr<Group const> group =
    getExistingGroup(g->productDescription().branchID());
  if (!group) {
    addGroup_(std::move(g));
    return;
  }
  BranchDescription const& bd = group->productDescription();
  mf::LogWarning("ResultsMerging")
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
art::ResultsPrincipal::
addGroup(BranchDescription const& bd)
{
  addOrReplaceGroup(gfactory::make_group(bd,
                                         ProductID{},
                                         RangeSet::invalid()));
}

void
art::ResultsPrincipal::
addGroup(std::unique_ptr<EDProduct>&& prod, BranchDescription const& bd)
{
  addOrReplaceGroup(gfactory::make_group(bd,
                                         ProductID{},
                                         RangeSet::invalid(),
                                         std::move(prod)));
}

void
art::ResultsPrincipal::
put(std::unique_ptr<EDProduct>&& edp, BranchDescription const& bd,
    std::unique_ptr<ProductProvenance const>&& productProvenance)
{
  if (!edp) {
    throw art::Exception(art::errors::ProductPutFailure, "Null Pointer")
        << "put: Cannot put because unique_ptr to product is null."
        << "\n";
  }
  branchMapper().insert(std::move(productProvenance));
  this->addGroup(std::move(edp), bd);
}
