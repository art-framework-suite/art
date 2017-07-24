#include "art/Framework/Principal/EventPrincipal.h"
// vim: set sw=2:

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "cetlib/container_algorithms.h"

#include <algorithm>
#include <utility>

using namespace cet;
using namespace std;

namespace art {

  EventPrincipal::EventPrincipal(EventAuxiliary const& aux,
                                 ProcessConfiguration const& pc,
                                 std::shared_ptr<History> history,
                                 std::unique_ptr<BranchMapper>&& mapper,
                                 std::unique_ptr<DelayedReader>&& rtrv,
                                 bool const lastInSubRun,
                                 int const idx)
  : Principal{pc, history->processHistoryID(), std::move(mapper), std::move(rtrv), idx}
  , aux_{aux}
  , history_{history}
  , lastInSubRun_{lastInSubRun}
  {
    productReader().setGroupFinder(cet::exempt_ptr<EDProductGetterFinder const>{this});
    if (ProductMetaData::instance().productProduced(InEvent)) {
      addToProcessHistory();
    }
  }

  SubRunPrincipal const&
  EventPrincipal::subRunPrincipal() const
  {
    if (!subRunPrincipal_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL subRunPrincipal.\n";
    }
    return *subRunPrincipal_;
  }

  void
  EventPrincipal::throwIfExistingGroup(BranchDescription const& pd) const
  {
    if (getGroup(pd.productID()) != nullptr) {
      throw art::Exception(art::errors::ProductRegistrationFailure, "EventPrincipal::throwIfExistingGroup")
        << "Problem found while adding product provenance: "
        << "product already exists for ("
        << pd.friendlyClassName()
        << ","
        << pd.moduleLabel()
        << ","
        << pd.productInstanceName()
        << ","
        << pd.processName()
        << ","
        << pd.branchType()
        << ")\n";
    }
  }

  void
  EventPrincipal::fillGroup(BranchDescription const& pd)
  {
    throwIfExistingGroup(pd);
    Principal::fillGroup(gfactory::make_group(pd,
                                              pd.productID(),
                                              RangeSet::invalid()));
  }

  void
  EventPrincipal::put(std::unique_ptr<EDProduct>&& edp,
                      BranchDescription const& pd,
                      std::unique_ptr<ProductProvenance const>&& productProvenance)
  {
    assert(edp);
    branchMapper().insert(std::move(productProvenance));
    throwIfExistingGroup(pd);
    Principal::fillGroup(gfactory::make_group(pd,
                                              pd.productID(),
                                              RangeSet::invalid(),
                                              std::move(edp)));
  }

  EventSelectionIDVector const&
  EventPrincipal::eventSelectionIDs() const
  {
    return history_->eventSelectionIDs();
  }

} // namespace art
