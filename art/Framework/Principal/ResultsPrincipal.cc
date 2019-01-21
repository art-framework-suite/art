#include "art/Framework/Principal/ResultsPrincipal.h"
// vim: set sw=2:

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "canvas/Persistency/Provenance/ProductID.h"

art::ResultsPrincipal::ResultsPrincipal(
  ResultsAuxiliary const& aux,
  ProcessConfiguration const& pc,
  cet::exempt_ptr<ProductTable const> presentProducts,
  bool const parentageEnabled,
  bool const rangesEnabled,
  std::unique_ptr<BranchMapper>&& mapper,
  std::unique_ptr<DelayedReader>&& rtrv)
  : Principal{pc,
              aux.processHistoryID_,
              presentProducts,
              std::move(mapper),
              std::move(rtrv)}
  , aux_{aux}
  , parentageEnabled_{parentageEnabled}
  , rangesEnabled_{rangesEnabled}
{
  productReader().setGroupFinder(
    cet::exempt_ptr<EDProductGetterFinder const>{this});
}

art::BranchType
art::ResultsPrincipal::branchType() const
{
  return InResults;
}

art::ProcessHistoryID const&
art::ResultsPrincipal::processHistoryID() const
{
  return aux().processHistoryID_;
}

void
art::ResultsPrincipal::setProcessHistoryID(ProcessHistoryID const& phid)
{
  aux().setProcessHistoryID(phid);
}

void
art::ResultsPrincipal::fillGroup(BranchDescription const& pd)
{
  Principal::fillGroup(
    gfactory::make_group(pd, pd.productID(), RangeSet::invalid()));
}

void
art::ResultsPrincipal::put(
  std::unique_ptr<EDProduct>&& edp,
  BranchDescription const& pd,
  std::unique_ptr<ProductProvenance const>&& productProvenance)
{
  assert(edp);
  branchMapper().insert(std::move(productProvenance));
  Principal::fillGroup(gfactory::make_group(
    pd, pd.productID(), RangeSet::invalid(), std::move(edp)));
}
