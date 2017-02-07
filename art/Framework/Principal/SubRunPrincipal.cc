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
                  int idx,
                  SubRunPrincipal* primaryPrincipal)
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
  SubRunPrincipal::
  addGroup(BranchDescription const& bd)
  {
    addOrReplaceGroup(gfactory::make_group(bd,
                                           ProductID{},
                                           RangeSet::invalid()));
  }

  void
  SubRunPrincipal::
  addGroup(std::unique_ptr<EDProduct>&& prod,
           BranchDescription const& bd,
           RangeSet&& rs)
  {
    addOrReplaceGroup(gfactory::make_group(bd,
                                           ProductID{},
                                           std::move(rs),
                                           std::move(prod)));
  }

  void
  SubRunPrincipal::
  put(std::unique_ptr<EDProduct>&& edp,
      BranchDescription const& bd,
      std::unique_ptr<ProductProvenance const>&& productProvenance,
      RangeSet&& rs)
  {
    if (!edp) {
      throw art::Exception(art::errors::ProductPutFailure, "Null Pointer")
        << "put: Cannot put because unique_ptr to product is null."
        << "\n";
    }
    branchMapper().insert(std::move(productProvenance));
    addGroup(std::move(edp), bd, std::move(rs));
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
