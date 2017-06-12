#include "art/Framework/Principal/EventPrincipal.h"
// vim: set sw=2:

#include "art/Framework/Principal/DeferredProductGetter.h"
#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Persistency/Provenance/BranchListIndex.h"
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
  productReader().setGroupFinder(cet::exempt_ptr<EDProductGetterFinder const>(this));
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
  if (auto group = getExistingGroup(bd.branchID())) {
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
                                            ProductID{bd.branchID().id()},
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
                                            ProductID{bd.branchID().id()},
                                            RangeSet::invalid(),
                                            std::move(edp)));
}

EDProductGetter const*
EventPrincipal::productGetter(ProductID const& pid) const
{
  EDProductGetter const* result{getByProductID(pid).result().get()};
  return result ? result : deferredGetter_(pid);
}

GroupQueryResult
EventPrincipal::getGroup(ProductID const& pid) const
{
  BranchID const bid{pid.value()};
  if (auto const g = getGroupForPtr(art::InEvent, bid)) {
    return GroupQueryResult{g};
  }
  auto whyFailed = std::make_shared<art::Exception>(art::errors::ProductNotFound, "InvalidID");
  *whyFailed << "getGroup: no product with given product id: " << pid << "\n";
  return GroupQueryResult{whyFailed};
}

GroupQueryResult
EventPrincipal::getByProductID(ProductID const& pid) const
{
  return getGroup(pid);
}

EventSelectionIDVector const&
EventPrincipal::eventSelectionIDs() const
{
  return history_->eventSelectionIDs();
}

EDProductGetter const*
EventPrincipal::deferredGetter_(ProductID const& pid) const
{
  auto it = deferredGetters_.find(pid);
  if (it != deferredGetters_.end()) {
    return it->second.get();
  }
  deferredGetters_[pid] = std::make_shared<DeferredProductGetter>(cet::exempt_ptr<EventPrincipal const>{this}, pid);
  return deferredGetters_[pid].get();
}

} // namespace art
