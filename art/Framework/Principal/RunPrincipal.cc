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
               EventRangeHandler const& erh,
               std::unique_ptr<BranchMapper>&& mapper,
               std::unique_ptr<DelayedReader>&& rtrv,
               int idx,
               RunPrincipal* primaryPrincipal)
    : Principal{pc, aux.processHistoryID_, std::move(mapper), std::move(rtrv), idx, primaryPrincipal}
    , aux_{aux}
    , rangeSetHandler_{erh}
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
  setOutputEventRanges(RangeSet const& outputRanges)
  {
    rangeSetHandler_.setOutputRanges(outputRanges);
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
                                           ProductID(),
                                           productRangeSetLookup()));
  }

  void
  RunPrincipal::
  addGroup(std::unique_ptr<EDProduct>&& prod, BranchDescription const& bd)
  {
    addOrReplaceGroup(gfactory::make_group(std::move(prod),
                                           bd,
                                           ProductID(),
                                           productRangeSetLookup()));
  }

  void
  RunPrincipal::
  put(std::unique_ptr<EDProduct>&& edp,
      BranchDescription const& bd,
      std::unique_ptr<ProductProvenance const>&& productProvenance)
  {
    if (!edp) {
      throw Exception(errors::InsertFailure, "Null Pointer")
        << "put: Cannot put because unique_ptr to product is null."
        << "\n";
    }
    branchMapper().insert(std::move(productProvenance));
    addGroup(std::move(edp), bd);
  }

} // namespace art
