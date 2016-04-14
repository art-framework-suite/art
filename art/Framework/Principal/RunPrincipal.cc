#include "art/Framework/Principal/RunPrincipal.h"
// vim: set sw=2:

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace art {

  using RangeSetHandler = detail::RangeSetHandler;

  RunPrincipal::
  RunPrincipal(RunAuxiliary const& aux,
               ProcessConfiguration const& pc,
               std::unique_ptr<RangeSetHandler>&& rsh,
               std::unique_ptr<BranchMapper>&& mapper,
               std::unique_ptr<DelayedReader>&& rtrv,
               int idx,
               RunPrincipal* primaryPrincipal)
    : Principal{pc, aux.processHistoryID_, std::move(mapper), std::move(rtrv), idx, primaryPrincipal}
    , aux_{aux}
    , rangeSetHandler_{std::move(rsh)}
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
  addOrReplaceGroup(std::unique_ptr<Group>&& g)
  {
    cet::exempt_ptr<Group const> group = getExistingGroup(g->productDescription().branchID());
    if (!group) {
      addGroup_(std::move(g));
    }
    else {
      replaceGroup(std::move(g));
    }
  }

  void
  RunPrincipal::
  addGroup(BranchDescription const& bd)
  {
    addOrReplaceGroup(gfactory::make_group(bd,
                                           ProductID{},
                                           RangeSet::invalid()));
  }

  void
  RunPrincipal::
  addGroup(std::unique_ptr<EDProduct>&& prod,
           BranchDescription const& bd,
           RangeSet&& rs)
  {
    addOrReplaceGroup(gfactory::make_group(std::move(prod),
                                           bd,
                                           ProductID{},
                                           std::move(rs)));
  }

  void
  RunPrincipal::
  put(std::unique_ptr<EDProduct>&& edp,
      BranchDescription const& bd,
      std::unique_ptr<ProductProvenance const>&& productProvenance,
      RangeSet&& rs)
  {
    if (!edp) {
      throw Exception(errors::InsertFailure, "Null Pointer")
        << "put: Cannot put because unique_ptr to product is null."
        << "\n";
    }
    branchMapper().insert(std::move(productProvenance));
    addGroup(std::move(edp), bd, std::move(rs));
  }

  RangeSetHandler const&
  RunPrincipal::
  rangeSetHandler() const
  {
    if (!rangeSetHandler_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL rangeSetHandler.\n";
    }
    return *rangeSetHandler_;
  }

  RangeSetHandler&
  RunPrincipal::
  rangeSetHandler()
  {
    if (!rangeSetHandler_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL rangeSetHandler.\n";
    }
    return *rangeSetHandler_;
  }

} // namespace art
