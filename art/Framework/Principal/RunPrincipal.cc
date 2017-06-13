#include "art/Framework/Principal/RunPrincipal.h"
// vim: set sw=2:

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace art {

  RunPrincipal::
  RunPrincipal(RunAuxiliary const& aux,
               ProcessConfiguration const& pc,
               std::unique_ptr<BranchMapper>&& mapper,
               std::unique_ptr<DelayedReader>&& rtrv,
               int const idx,
               cet::exempt_ptr<RunPrincipal const> primaryPrincipal)
    : Principal{pc, aux.processHistoryID_, std::move(mapper), std::move(rtrv), idx, primaryPrincipal}
    , aux_{aux}
  {
    if (ProductMetaData::instance().productProduced(InRun)) {
      addToProcessHistory();
    }
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
  fillGroup(BranchDescription const& bd)
  {
    Principal::fillGroup(gfactory::make_group(bd,
                                              ProductID{bd.branchID().id()},
                                              RangeSet::invalid()));
  }

  void
  RunPrincipal::
  put(std::unique_ptr<EDProduct>&& edp,
      BranchDescription const& bd,
      std::unique_ptr<ProductProvenance const>&& productProvenance,
      RangeSet&& rs)
  {
    assert(edp);
    branchMapper().insert(std::move(productProvenance));
    Principal::fillGroup(gfactory::make_group(bd,
                                              ProductID{bd.branchID().id()},
                                              std::move(rs),
                                              std::move(edp)));

  }

} // namespace art
