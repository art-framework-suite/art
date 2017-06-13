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
  SubRunPrincipal(SubRunAuxiliary const& aux,
                  ProcessConfiguration const& pc,
                  std::unique_ptr<BranchMapper>&& mapper,
                  std::unique_ptr<DelayedReader>&& rtrv,
                  int const idx,
                  cet::exempt_ptr<SubRunPrincipal const> primaryPrincipal)
    : Principal{pc, aux.processHistoryID_, std::move(mapper), std::move(rtrv), idx, primaryPrincipal}
    , aux_{aux}
  {
    if (ProductMetaData::instance().productProduced(InSubRun)) {
      addToProcessHistory();
    }
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
  fillGroup(BranchDescription const& bd)
  {
    Principal::fillGroup(gfactory::make_group(bd,
                                              bd.productID(),
                                              RangeSet::invalid()));
  }

  void
  SubRunPrincipal::
  put(std::unique_ptr<EDProduct>&& edp,
      BranchDescription const& bd,
      std::unique_ptr<ProductProvenance const>&& productProvenance,
      RangeSet&& rs)
  {
    assert(edp);
    branchMapper().insert(std::move(productProvenance));
    Principal::fillGroup(gfactory::make_group(bd,
                                              bd.productID(),
                                              std::move(rs),
                                              std::move(edp)));
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

} // namespace art
