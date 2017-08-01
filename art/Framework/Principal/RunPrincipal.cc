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
               BranchTypeLookups const& productLookup,
               BranchTypeLookups const& elementLookup,
               std::unique_ptr<BranchMapper>&& mapper,
               std::unique_ptr<DelayedReader>&& rtrv)
    : Principal{pc, aux.processHistoryID_, productLookup, elementLookup, std::move(mapper), std::move(rtrv)}
    , aux_{aux}
  {
    productReader().setGroupFinder(cet::exempt_ptr<EDProductGetterFinder const>{this});
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
  fillGroup(BranchDescription const& pd)
  {
    Principal::fillGroup(gfactory::make_group(pd,
                                              pd.productID(),
                                              RangeSet::invalid()));
  }

  void
  RunPrincipal::
  put(std::unique_ptr<EDProduct>&& edp,
      BranchDescription const& pd,
      std::unique_ptr<ProductProvenance const>&& productProvenance,
      RangeSet&& rs)
  {
    assert(edp);
    branchMapper().insert(std::move(productProvenance));
    Principal::fillGroup(gfactory::make_group(pd,
                                              pd.productID(),
                                              std::move(rs),
                                              std::move(edp)));

  }

} // namespace art
