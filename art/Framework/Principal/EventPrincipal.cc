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
                                 int const idx,
                                 cet::exempt_ptr<EventPrincipal const> primaryPrincipal)
  : Principal{pc, history->processHistoryID(), std::move(mapper), std::move(rtrv), idx, primaryPrincipal}
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
  EventPrincipal::throwIfExistingGroup(BranchDescription const& bd) const
  {
    if (auto group = getExistingGroup(bd.productID())) {
      throw art::Exception(art::errors::ProductRegistrationFailure, "EventPrincipal::throwIfExistingGroup")
        << "Problem found while adding product provenance: "
        << "product already exists for ("
        << bd.friendlyClassName()
        << ","
        << bd.moduleLabel()
        << ","
        << bd.productInstanceName()
        << ","
        << bd.processName()
        << ","
        << bd.branchType()
        << ")\n";
    }
  }

  void
  EventPrincipal::fillGroup(BranchDescription const& bd)
  {
    throwIfExistingGroup(bd);
    Principal::fillGroup(gfactory::make_group(bd,
                                              bd.productID(),
                                              RangeSet::invalid()));
  }

  void
  EventPrincipal::put(std::unique_ptr<EDProduct>&& edp,
                      BranchDescription const& bd,
                      std::unique_ptr<ProductProvenance const>&& productProvenance)
  {
    assert(edp);
    branchMapper().insert(std::move(productProvenance));
    throwIfExistingGroup(bd);
    Principal::fillGroup(gfactory::make_group(bd,
                                              bd.productID(),
                                              RangeSet::invalid(),
                                              std::move(edp)));
  }

  EventSelectionIDVector const&
  EventPrincipal::eventSelectionIDs() const
  {
    return history_->eventSelectionIDs();
  }

} // namespace art
