#include "art/Framework/Principal/ResultsPrincipal.h"
// vim: set sw=2:

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

art::ResultsPrincipal::
ResultsPrincipal(ResultsAuxiliary const& aux,
                 ProcessConfiguration const& pc,
                 std::unique_ptr<BranchMapper>&& mapper,
                 std::unique_ptr<DelayedReader>&& rtrv,
                 int const idx)
  : Principal{pc, aux.processHistoryID_, std::move(mapper), std::move(rtrv), idx}
  , aux_{aux}
{
  productReader().setGroupFinder(cet::exempt_ptr<EDProductGetterFinder const>{this});
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
fillGroup(BranchDescription const& pd)
{
  Principal::fillGroup(gfactory::make_group(pd,
                                            pd.productID(),
                                            RangeSet::invalid()));
}

void
art::ResultsPrincipal::
put(std::unique_ptr<EDProduct>&& edp,
    BranchDescription const& pd,
    std::unique_ptr<ProductProvenance const>&& productProvenance)
{
  assert(edp);
  branchMapper().insert(std::move(productProvenance));
  Principal::fillGroup(gfactory::make_group(pd,
                                            pd.productID(),
                                            RangeSet::invalid(),
                                            std::move(edp)));
}
