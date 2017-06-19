#include "art/Framework/Principal/Principal.h"
// vim: set sw=2:

#include "art/Framework/Principal/DeferredProductGetter.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Persistency/Provenance/TypeTools.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "cetlib/container_algorithms.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <utility>

using namespace cet;
using namespace std;

namespace art {

Principal::
Principal(ProcessConfiguration const& pc,
          ProcessHistoryID const& hist,
          std::unique_ptr<BranchMapper>&& mapper,
          std::unique_ptr<DelayedReader>&& reader,
          int const idx,
          cet::exempt_ptr<Principal const> primaryPrincipal)
  : processConfiguration_{pc}
  , branchMapperPtr_{std::move(mapper)}
  , store_{std::move(reader)}
  , primaryPrincipal_{primaryPrincipal}
  , secondaryIdx_{idx}
{
  if (!hist.isValid()) {
    return;
  }
  assert(!ProcessHistoryRegistry::empty());
  ProcessHistory ph;
  bool const found [[gnu::unused]] {ProcessHistoryRegistry::get(hist, ph)};
  assert(found);
  std::swap(*processHistoryPtr_ , ph);
}

void
Principal::
addToProcessHistory()
{
  if (processHistoryModified_) {
    return;
  }
  ProcessHistory& ph = *processHistoryPtr_;
  string const& processName = processConfiguration_.processName();
  for (auto const& val : ph) {
    if (processName == val.processName()) {
      throw art::Exception(errors::Configuration)
          << "The process name "
          << processName
          << " was previously used on these products.\n"
          << "Please modify the configuration file to use a "
          << "distinct process name.\n";
    }
  }
  ph.push_back(processConfiguration_);
  // OPTIMIZATION NOTE: As of 0_9_0_pre3
  // For very simple Sources (e.g. EmptyEvent) this routine takes up
  // nearly 50% of the time per event, and 96% of the time for this
  // routine is spent in computing the ProcessHistory id which happens
  // because we are reconstructing the ProcessHistory for each event.
  // It would probably be better to move the ProcessHistory
  // construction out to somewhere which persists for longer than one
  // Event.
  ProcessHistoryRegistry::emplace(ph.id(), ph);
  setProcessHistoryID(ph.id());
  processHistoryModified_ = true;
}

GroupQueryResult
Principal::getBySelector(TypeID const& productType, SelectorBase const& sel) const
{
  GroupQueryResultVec results;
  int nFound = findGroupsForProduct(productType, sel, results, true);
  if (nFound == 0) {
    auto whyFailed = std::make_shared<art::Exception>(art::errors::ProductNotFound);
    *whyFailed << "getBySelector: Found zero products matching all criteria\n"
               << "Looking for type: "
               << productType
               << "\n";
    return GroupQueryResult(whyFailed);
  }
  if (nFound > 1) {
    throw art::Exception(art::errors::ProductNotFound)
        << "getBySelector: Found "
        << nFound
        << " products rather than one which match all criteria\n"
        << "Looking for type: "
        << productType
        << "\n";
  }
  return results[0];
}

GroupQueryResult
Principal::getByProductID(ProductID const pid) const
{
  if (auto const g = getGroupForPtr(art::InEvent, pid)) {
    return GroupQueryResult{g};
  }
  auto whyFailed = std::make_shared<art::Exception>(art::errors::ProductNotFound, "InvalidID");
  *whyFailed << "getGroup: no product with given product id: " << pid << "\n";
  return GroupQueryResult{whyFailed};
}

GroupQueryResult
Principal::getByLabel(TypeID const& productType,
                      string const& label,
                      string const& productInstanceName,
                      string const& processName) const
{
  GroupQueryResultVec results;
  Selector sel(ModuleLabelSelector{label} &&
               ProductInstanceNameSelector{productInstanceName} &&
               ProcessNameSelector{processName});
  int const nFound = findGroupsForProduct(productType, sel, results, true);
  if (nFound == 0) {
    auto whyFailed = std::make_shared<art::Exception>(art::errors::ProductNotFound);
    *whyFailed << "getByLabel: Found zero products matching all criteria\n"
               << "Looking for type: "
               << productType
               << "\n"
               << "Looking for module label: "
               << label
               << "\n"
               << "Looking for productInstanceName: "
               << productInstanceName
               << "\n"
               << (processName.empty() ? "" : "Looking for process: ")
               << processName
               << "\n";
    return GroupQueryResult(whyFailed);
  }
  if (nFound > 1) {
    throw art::Exception(art::errors::ProductNotFound)
        << "getByLabel: Found "
        << nFound
        << " products rather than one which match all criteria\n"
        << "Looking for type: "
        << productType
        << "\n"
        << "Looking for module label: "
        << label
        << "\n"
        << "Looking for productInstanceName: "
        << productInstanceName
        << "\n"
        << (processName.empty() ? "" : "Looking for process: ")
        << processName
        << "\n";
  }
  return results[0];
}

void
Principal::
getMany(TypeID const& productType, SelectorBase const& sel,
        GroupQueryResultVec& results) const
{
  findGroupsForProduct(productType, sel, results, false);
}

int
Principal::
tryNextSecondaryFile() const
{
  int const err = store_->openNextSecondaryFile(nextSecondaryFileIdx_);
  if (err != -2) {
    // there are more files to try
    ++nextSecondaryFileIdx_;
  }
  return err;
}

size_t
Principal::
getMatchingSequence(TypeID const& elementType, SelectorBase const& selector,
                    GroupQueryResultVec& results,
                    bool stopIfProcessHasMatch) const
{
  for (auto const& el : ProductMetaData::instance().elementLookup()) {
    auto I = el[branchType()].find(elementType.friendlyClassName());
    if (I == el[branchType()].end()) {
      continue;
    }
    if (auto ret = findGroups(I->second, selector, results, stopIfProcessHasMatch)) {
      return ret;
    }
  }
  while (1) {
    int err = tryNextSecondaryFile();
    if (err == -2) {
      // No more files.
      break;
    }
    if (err == -1) {
      // Run, SubRun, or Event not found.
      continue;
    }
    // Note: The elementLookup vector element zero is for the primary
    // file, the following elements are for the secondary files, so
    // we use the incremented value of nextSecondaryFileIdx_ here
    // because it is the correctly biased-up by one index into the
    // elementLookup vector for this secondary file.
    auto const& el = ProductMetaData::instance().elementLookup()[nextSecondaryFileIdx_];
    auto I = el[branchType()].find(elementType.friendlyClassName());
    if (I == el[branchType()].end()) {
      continue;
    }
    if (auto ret = findGroups(I->second, selector, results, stopIfProcessHasMatch) ) {
      return ret;
    }
  }
  return 0;
}

void
Principal::
removeCachedProduct(ProductID const pid) const
{
  if (auto g = getExistingGroup(pid)) {
    g->removeCachedProduct();
    return;
  }
  throw Exception(errors::ProductNotFound, "removeCachedProduct")
    << "Attempt to remove unknown product corresponding to ProductID: " << pid << '\n'
    << "Please contact artists@fnal.gov\n";
}

size_t
Principal::
findGroupsForProduct(TypeID const& wanted_product,
                     SelectorBase const& selector,
                     GroupQueryResultVec& results,
                     bool stopIfProcessHasMatch) const
{
  TClass * cl = nullptr;
  ////////////////////////////////////
  // Cannot do this here because some tests expect to be able to
  // call here requesting a wanted_product that does not have
  // a dictionary and be ok because the productLookup fails.
  // See issue #8532.
  //cl = TClass::GetClass(wrappedClassName(wanted_product.className()).c_str());
  //if (!cl) {
  //  throw Exception(errors::DictionaryNotFound)
  //      << "Dictionary not found for "
  //      << wrappedClassName(wanted_product.className())
  //      << ".\n";
  //}
  ////////////////////////////////////
  size_t ret = 0;
  for (auto const& pl : ProductMetaData::instance().productLookup()) {
    auto I = pl[branchType()].find(wanted_product.friendlyClassName());
    if (I == pl[branchType()].end()) {
      continue;
    }
    cl = TClass::GetClass(wrappedClassName(wanted_product.className()).c_str());
    if (!cl) {
      throw Exception(errors::DictionaryNotFound)
        << "Dictionary not found for "
        << wrappedClassName(wanted_product.className())
        << ".\n";
    }
    ret = findGroups(I->second, selector, results, stopIfProcessHasMatch,
                     TypeID(cl->GetTypeInfo()));
    if (ret) {
      return ret;
    }
  }
  while (1) {
    int err = tryNextSecondaryFile();
    if (err == -2) {
      // No more files.
      break;
    }
    if (err == -1) {
      // Run, SubRun, or Event not found.
      continue;
    }
    // Note: The productLookup vector element zero is for the primary
    // file, the following elements are for the secondary files, so
    // we use the incremented value of nextSecondaryFileIdx_ here
    // because it is the correctly biased-up by one index into the
    // productLookup vector for this secondary file.
    auto const& pl =
      ProductMetaData::instance().productLookup()[nextSecondaryFileIdx_];
    auto I = pl[branchType()].find(wanted_product.friendlyClassName());
    if (I == pl[branchType()].end()) {
      continue;
    }
    cl = TClass::GetClass(wrappedClassName(wanted_product.className()).c_str());
    if (!cl) {
      throw Exception(errors::DictionaryNotFound)
    << "Dictionary not found for "
    << wrappedClassName(wanted_product.className())
    << ".\n";
    }
    ret = findGroups(I->second, selector, results, stopIfProcessHasMatch,
                     TypeID(cl->GetTypeInfo()));
    if (ret) {
      return ret;
    }
  }
  return 0;
}

size_t
Principal::
findGroups(ProcessLookup const& pl, SelectorBase const& sel,
           GroupQueryResultVec& res, bool stopIfProcessHasMatch,
           TypeID wanted_wrapper/*=TypeID()*/) const
{
  // Handle groups for current process, note that we need to look at
  // the current process even if it is not in the processHistory
  // because of potential unscheduled (onDemand) production.
  // [We don't support unscheduled production anymore, so I'm not sure
  // how much the logic here should be adjusted. -KJK]
  {
    auto I = pl.find(processConfiguration_.processName());
    if (I != pl.end()) {
      findGroupsForProcess(I->second, sel, res, wanted_wrapper);
    }
  }
  // Loop over processes in reverse time order.  Sometimes we want to stop
  // after we find a process with matches so check for that at each step.
  for (auto I = processHistory().crbegin(), E = processHistory().crend();
       (I != E) && (res.empty() || !stopIfProcessHasMatch); ++I) {
    // We just dealt with the current process before the loop so skip it
    if (I->processName() == processConfiguration_.processName()) {
      continue;
    }
    auto J = pl.find(I->processName());
    if (J != pl.end()) {
      findGroupsForProcess(J->second, sel, res, wanted_wrapper);
    }
  }
  return res.size();
}

void
Principal::
findGroupsForProcess(std::vector<ProductID> const& vpid,
                     SelectorBase const& sel,
                     GroupQueryResultVec& res,
                     TypeID wanted_wrapper) const
{

  for (auto const pid : vpid) {
    auto group = getGroup(pid);
    if (!group) {
      continue;
    }
    if (!sel.match(group->productDescription())) {
      continue;
    }
    if (group->productUnavailable()) {
      continue;
    }
    if (wanted_wrapper) {
      group->resolveProduct(wanted_wrapper);
    }
    else {
      group->resolveProduct(group->producedWrapperType());
    }
    // If the product is a dummy filler, group will now be marked unavailable.
    // Unscheduled execution can fail to produce the EDProduct so check.
    if (group->productUnavailable()) {
      continue;
    }
    // Found a good match, save it.
    res.emplace_back(group);
  }
}

EDProductGetter const*
Principal::deferredGetter_(ProductID const pid) const
{
  auto it = deferredGetters_.find(pid);
  if (it != deferredGetters_.end()) {
    return it->second.get();
  }
  deferredGetters_[pid] = std::make_shared<DeferredProductGetter>(cet::exempt_ptr<Principal const>{this}, pid);
  return deferredGetters_[pid].get();
}

OutputHandle
Principal::getForOutput(ProductID const pid, bool resolveProd) const
{
  auto const& g = getResolvedGroup(pid, resolveProd);
  if (g.get() == nullptr) {
    return OutputHandle{RangeSet::invalid()};
  }
  auto const& pmd = ProductMetaData::instance();
  auto const bt = g->productDescription().branchType();
  if (resolveProd
      &&
      ((g->anyProduct() == nullptr) || !g->anyProduct()->isPresent())
      &&
      (pmd.presentWithFileIdx(bt, pid) != MasterProductRegistry::DROPPED ||
       pmd.produced(bt, pid))
      &&
      (bt == InEvent)
      &&
      productstatus::present(g->productProvenancePtr()->productStatus())) {
    throw Exception(errors::LogicError, "Principal::getForOutput\n")
      << "A product with a status of 'present' is not actually present.\n"
      << "The branch name is "
      << g->productDescription().branchName()
      << "\nContact a framework developer.\n";
  }
  if (!g->anyProduct() && !g->productProvenancePtr()) {
    return OutputHandle{g->rangeOfValidity()};
  }
  return OutputHandle{g->anyProduct(), &g->productDescription(), g->productProvenancePtr(), g->rangeOfValidity()};
}

cet::exempt_ptr<Group const> const
Principal::
getResolvedGroup(ProductID const pid,
                 bool const resolveProd) const
{
  // FIXME: This reproduces the behavior of the original getGroup with
  // resolveProv == false but I am not sure this is correct in the case
  // of an unavailable product.
  auto const g = getGroupForPtr(branchType(),pid);
  if (!g.get() || !resolveProd) {
    return g;
  }
  bool const gotIt = g->resolveProductIfAvailable(g->producedWrapperType());
  if (!gotIt) {
    // Behavior is the same as if the group wasn't there.
    return nullptr;
  }
  return g;
}

cet::exempt_ptr<Group const>
Principal::
getExistingGroup(ProductID const pid) const
{
  auto I = groups_.find(pid);
  if (I != groups_.end()) {
    return I->second.get();
  }
  for (auto& p : secondaryPrincipals_) {
    auto I = p->groups_.find(pid);
    if (I != p->groups_.end()) {
      return I->second.get();
    }
  }
  return nullptr;
}

cet::exempt_ptr<Group const> const
Principal::
getGroupForPtr(BranchType const btype, ProductID const pid) const
{
  Principal const* pp = this;
  if (primaryPrincipal_ != nullptr) {
    pp = primaryPrincipal_.get();
  }

  std::size_t const index    = ProductMetaData::instance().presentWithFileIdx(btype, pid);
  bool        const produced = ProductMetaData::instance().produced(btype, pid);
  if ( produced || index == 0 ) {
    auto it = pp->groups_.find(pid);
    // Note: There will be groups for dropped products, so we
    //       must check for that.  We want the group where the
    //       product can actually be retrieved from.
    return (it != pp->groups_.end()) ? it->second.get() : nullptr;
  }
  else if ( index > 0 && ( index-1 < secondaryPrincipals_.size() ) ) {
    auto const& groups = secondaryPrincipals_[index-1]->groups_;
    auto it = groups.find(pid);
    return (it != groups.end()) ? it->second.get() : nullptr;
  }
  while (1) {
    int err = tryNextSecondaryFile();
    if (err == -2) {
      // No more files.
      return nullptr;
    }
    if (err == -1) {
      // Run, SubRun, or Event not found.
      continue;
    }
    std::size_t const index = ProductMetaData::instance().presentWithFileIdx(btype, pid);
    if (index == MasterProductRegistry::DROPPED) continue;
    auto& p = secondaryPrincipals_[index-1];
    auto it = p->groups_.find(pid);
    // Note: There will be groups for dropped products, so we
    //       must check for that.  We want the group where the
    //       product can actually be retrieved from.
    return (it != p->groups_.end()) ? it->second.get() : nullptr;
  }
}

cet::exempt_ptr<Group const> const
Principal::
getGroup(ProductID const pid) const
{
  const Principal* pp = this;
  if (primaryPrincipal_ != nullptr) {
    pp = primaryPrincipal_.get();
  }
  {
    auto I = pp->groups_.find(pid);
    if (I != pp->groups_.end()) {
      return I->second.get();
    }
  }
  for (auto& p : secondaryPrincipals_) {
    auto I = p->groups_.find(pid);
    if (I != p->groups_.end()) {
      return I->second.get();
    }
  }
  return nullptr;
}

EDProductGetter const*
Principal::productGetter(ProductID const pid) const
{
  EDProductGetter const* result{getByProductID(pid).result().get()};
  return result ? result : deferredGetter_(pid);
}

} // namespace art
