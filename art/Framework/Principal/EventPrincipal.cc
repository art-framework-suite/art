#include "art/Framework/Principal/EventPrincipal.h"
// vim: set sw=2:

#include "art/Framework/Principal/DeferredProductGetter.h"
#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/GroupFactory.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Persistency/Provenance/BranchListIndex.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "cetlib/container_algorithms.h"
#include "cpp0x/algorithm"
#include "cpp0x/utility"

using namespace cet;
using namespace std;

namespace art {

EventPrincipal::
~EventPrincipal()
{
}

EventPrincipal::
EventPrincipal(EventAuxiliary const& aux, ProcessConfiguration const& pc,
               std::shared_ptr<History> history,
               std::unique_ptr<BranchMapper>&& mapper,
               std::unique_ptr<DelayedReader>&& rtrv, int idx,
               EventPrincipal* primaryPrincipal)
  : Principal(pc, history->processHistoryID(), std::move(mapper),
              std::move(rtrv), idx, primaryPrincipal)
  , deferredGetters_()
  , aux_(aux)
  , subRunPrincipal_()
  , history_(history)
  , branchToProductIDHelper_()
{
  productReader().setGroupFinder(cet::exempt_ptr<EventPrincipal const>(this));
  if (ProductMetaData::instance().productProduced(InEvent)) {
    addToProcessHistory();
    // Add index into BranchIDListRegistry for products produced this process
    history_->addBranchListIndexEntry(
      BranchIDListRegistry::instance()->size() - 1);
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

SubRunPrincipal&
EventPrincipal::
subRunPrincipal()
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

RunPrincipal&
EventPrincipal::
runPrincipal()
{
  return subRunPrincipal().runPrincipal();
}

void
EventPrincipal::
addOrReplaceGroup(std::unique_ptr<Group>&& g)
{
  cet::exempt_ptr<Group const> group =
    getExistingGroup(g->productDescription().branchID());
  if (!group) {
    addGroup_(std::move(g));
    return;
  }
  if (group->onDemand()) {
    replaceGroup(std::move(g));
    return;
  }
  BranchDescription const& bd = group->productDescription();
  throw art::Exception(art::errors::InsertFailure, "AlreadyPresent")
      << "addGroup_: Problem found while adding product provenance, "
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

void
EventPrincipal::
addGroup(BranchDescription const& bd)
{
  addOrReplaceGroup(gfactory::make_group(bd,
                                         branchIDToProductID(bd.branchID())));
}

void
EventPrincipal::
addGroup(std::unique_ptr<EDProduct>&& prod, BranchDescription const& bd)
{
  addOrReplaceGroup(gfactory::make_group(std::move(prod),
                                         bd,
                                         branchIDToProductID(bd.branchID())));
}

void
EventPrincipal::
addOnDemandGroup(BranchDescription const& desc, cet::exempt_ptr<Worker> worker)
{
  ProductID pid(branchIDToProductID(desc.branchID()));
  cet::exempt_ptr<EventPrincipal> epp(this);
  addOrReplaceGroup(gfactory::make_group(desc, pid, worker, epp));
}

void
EventPrincipal::
put(std::unique_ptr<EDProduct>&& edp, BranchDescription const& bd,
    std::unique_ptr<ProductProvenance const>&& productProvenance)
{
  if (!edp) {
    throw art::Exception(art::errors::InsertFailure, "Null Pointer")
        << "put: Cannot put because unique_ptr to product is null.\n";
  }
  ProductID pid = branchIDToProductID(bd.branchID());
  if (!pid.isValid()) {
    throw art::Exception(art::errors::InsertFailure, "Null Product ID")
        << "put: Cannot put product with null Product ID.\n";
  }
  branchMapper().insert(std::move(productProvenance));
  this->addGroup(std::move(edp), bd);
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
  if (!pid.isValid()) {
    throw art::Exception(art::errors::ProductNotFound, "InvalidID")
        << "get by product ID: invalid ProductID supplied\n";
  }
  BranchID::value_type bid = 0;
  try {
    auto blix = history().branchListIndexes().at(pid.processIndex() - 1);
    auto const& blist = BranchIDListRegistry::instance()->data().at(blix);
    bid = blist.at(pid.productIndex() - 1);
  }
  catch (std::exception) {
    return BranchID();
  }
  return BranchID(bid);
}

ProductID
EventPrincipal::
branchIDToProductID(BranchID const& bid) const
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
  BranchID bid = productIDToBranchID(pid);
  SharedConstGroupPtr const& g = getGroupForPtr(art::InEvent,bid);
  if (g.get()) {
    return GroupQueryResult(g.get());
  }
  std::shared_ptr<cet::exception> whyFailed(
    new art::Exception(art::errors::ProductNotFound, "InvalidID"));
  *whyFailed
      << "getGroup: no product with given product id: " << pid << "\n";
  return GroupQueryResult(whyFailed);
}

GroupQueryResult
EventPrincipal::
getByProductID(ProductID const& pid) const
{
  // FIXME: This reproduces the logic of the old version of the
  // function, but I'm not sure it does the *right* thing in the face
  // of an unavailable product or other rare failure.
  BranchID bid = productIDToBranchID(pid);
  SharedConstGroupPtr const& g(getResolvedGroup(bid, true, true));
  if (!g) {
    std::shared_ptr<cet::exception>
    whyFailed(new art::Exception(art::errors::ProductNotFound, "InvalidID"));
    *whyFailed
        << "getGroup: no product with given product id: " << pid << "\n";
    return GroupQueryResult(whyFailed);
  }
  return GroupQueryResult(g.get());
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
  deferredGetters_[pid].reset(new DeferredProductGetter(
    cet::exempt_ptr<EventPrincipal const>(this), pid));
  return deferredGetters_[pid].get();
}

} // namespace art

