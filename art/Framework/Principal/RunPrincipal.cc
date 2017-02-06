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
               int idx,
               RunPrincipal* primaryPrincipal)
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
  addOrReplaceGroup(std::unique_ptr<Group>&& g)
  {
    if (getExistingGroup(g->productDescription().branchID()) != nullptr) {
      Principal::replaceGroup(std::move(g));
    }
    else {
      Principal::addGroup(std::move(g));
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
  put(std::unique_ptr<EDProduct>&& edp,
      BranchDescription const& bd,
      std::unique_ptr<ProductProvenance const>&& productProvenance,
      RangeSet&& rs)
  {
    assert(edp);
    branchMapper().insert(std::move(productProvenance));
    addOrReplaceGroup(gfactory::make_group(bd,
                                           ProductID{},
                                           std::move(rs),
                                           std::move(edp)));

  }

} // namespace art
