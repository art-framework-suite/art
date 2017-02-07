#include "art/Framework/Principal/EventPrincipal.h"
// vim: set sw=2:

#include "art/Framework/Principal/DeferredProductGetter.h"
#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Persistency/Provenance/BranchIDList.h"
#include "canvas/Persistency/Provenance/BranchListIndex.h"
#include "cetlib/container_algorithms.h"

#include <algorithm>
#include <utility>

using namespace cet;
using namespace std;

namespace art {

EventPrincipal::
EventPrincipal(EventAuxiliary const& aux,
               ProcessConfiguration const& pc,
               std::shared_ptr<History> history,
               std::unique_ptr<BranchMapper>&& mapper,
               std::unique_ptr<DelayedReader>&& rtrv,
               bool const lastInSubRun,
               int idx,
               EventPrincipal* primaryPrincipal)
  : Principal{pc, history->processHistoryID(), std::move(mapper), std::move(rtrv), idx, primaryPrincipal}
  , aux_{aux}
  , history_{history}
  , lastInSubRun_{lastInSubRun}
{
  productReader().setGroupFinder(cet::exempt_ptr<EDProductGetterFinder const>(this));
  if (ProductMetaData::instance().productProduced(InEvent)) {
    addToProcessHistory();
    // Add index into BranchIDListRegistry for products produced this process
    history_->addBranchListIndexEntry(BranchIDListRegistry::instance()->size() - 1);
  }
  // Fill in helper map for Branch to ProductID mapping
  for (auto IB = history->branchListIndexes().cbegin(),
       IE = history->branchListIndexes().cend(), I = IB; I != IE; ++I) {
    ProcessIndex pix = I - IB;
    branchToProductIDHelper_.insert({*I, pix});
  }
}

SubRunPrincipal const&
EventPrincipal::
subRunPrincipal() const
{
  if (!subRunPrincipal_) {
    throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL subRunPrincipal.\n";
  }
  return *subRunPrincipal_;
}

RunPrincipal const&
EventPrincipal::
runPrincipal() const
{
  return subRunPrincipal().runPrincipal();
}

void
EventPrincipal::
throwIfExistingGroup(BranchDescription const& bd) const
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
EventPrincipal::
addGroup(BranchDescription const& bd)
{
  throwIfExistingGroup(bd);
  Principal::addGroup(gfactory::make_group(bd,
                                           branchIDToProductID(bd.branchID()),
                                           RangeSet::invalid()));
}

void
EventPrincipal::
put(std::unique_ptr<EDProduct>&& edp,
    BranchDescription const& bd,
    std::unique_ptr<ProductProvenance const>&& productProvenance)
{
  assert(edp);
  branchMapper().insert(std::move(productProvenance));
  throwIfExistingGroup(bd);
  Principal::addGroup(gfactory::make_group(bd,
                                           branchIDToProductID(bd.branchID()),
                                           RangeSet::invalid(),
                                           std::move(edp)));
}

EDProductGetter const*
EventPrincipal::
productGetter(ProductID const& pid) const
{
  EDProductGetter const* result = getByProductID(pid).result().get();
  return result ? result : deferredGetter_(pid);
}

BranchID
EventPrincipal::
productIDToBranchID(ProductID const& pid) const
{
  BranchID result;
  if (!pid.isValid()) {
    throw art::Exception(art::errors::ProductNotFound, "InvalidID")
        << "get by product ID: invalid ProductID supplied\n";
  }
  auto procidx = pid.processIndex();
  if (procidx > 0) {
    --procidx;
    if (procidx < history().branchListIndexes().size()) {
      auto const blix = history().branchListIndexes()[procidx];
      if (blix < BranchIDListRegistry::instance()->data().size()) {
        auto const & blist = BranchIDListRegistry::instance()->data()[blix];
        auto productidx = pid.productIndex();
        if (productidx > 0) {
          --productidx;
          if (productidx < blist.size()) {
            result = BranchID(blist[productidx]);
          }
        }
      }
    }
  }
  return result;
}

ProductID
EventPrincipal::
branchIDToProductID(BranchID const bid) const
{
  if (!bid.isValid()) {
    throw art::Exception(art::errors::NotFound, "InvalidID")
        << "branchIDToProductID: invalid BranchID supplied\n";
  }
  auto const& branchIDToIndexMap =
    BranchIDListRegistry::instance()->extra().branchIDToIndexMap();
  auto it = branchIDToIndexMap.find(bid);
  if (it == branchIDToIndexMap.end()) {
    throw art::Exception(art::errors::NotFound, "Bad BranchID")
        << "branchIDToProductID: productID cannot be determined "
        "from BranchID\n";
  }
  auto blix = it->second.first;
  auto productIndex = it->second.second;
  auto i = branchToProductIDHelper_.find(blix);
  if (i == branchToProductIDHelper_.end()) {
    throw art::Exception(art::errors::NotFound, "Bad branch ID")
        << "branchIDToProductID: productID cannot be determined "
        "from BranchID\n";
  }
  auto processIndex = i->second;
  return ProductID(processIndex + 1, productIndex + 1);
}

GroupQueryResult
EventPrincipal::
getGroup(ProductID const& pid) const
{
  BranchID const bid = productIDToBranchID(pid);
  if (auto const g = getGroupForPtr(art::InEvent, bid)) {
    return GroupQueryResult{g};
  }
  auto whyFailed = std::make_shared<art::Exception>(art::errors::ProductNotFound, "InvalidID");
  *whyFailed << "getGroup: no product with given product id: " << pid << "\n";
  return GroupQueryResult{whyFailed};
}

GroupQueryResult
EventPrincipal::
getByProductID(ProductID const& pid) const
{
  // FIXME: This reproduces the logic of the old version of the
  // function, but I'm not sure it does the *right* thing in the face
  // of an unavailable product or other rare failure.
  BranchID const bid = productIDToBranchID(pid);
  if (auto const g = getResolvedGroup(bid, true)) {
    return GroupQueryResult{g};
  }
  auto whyFailed = std::make_shared<art::Exception>(art::errors::ProductNotFound, "InvalidID");
  *whyFailed << "getGroup: no product with given product id: " << pid << "\n";
  return GroupQueryResult{whyFailed};
}

EventSelectionIDVector const&
EventPrincipal::
eventSelectionIDs() const
{
  return history_->eventSelectionIDs();
}

EDProductGetter const*
EventPrincipal::
deferredGetter_(ProductID const& pid) const
{
  auto it = deferredGetters_.find(pid);
  if (it != deferredGetters_.end()) {
    return it->second.get();
  }
  deferredGetters_[pid] = std::make_shared<DeferredProductGetter>(cet::exempt_ptr<EventPrincipal const>{this}, pid);
  return deferredGetters_[pid].get();
}

} // namespace art
