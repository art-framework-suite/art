#include "art/Framework/Principal/Principal.h"
// vim: set sw=2:

#include "art/Framework/Principal/DeferredProductGetter.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <utility>

using namespace cet;
using namespace std;
using namespace art;

namespace {

  template <typename T>
  class ReverseIteration;

  template <typename T>
  ReverseIteration<T> reverse_iteration(T const&);

  template <typename T>
  class ReverseIteration {
    friend ReverseIteration reverse_iteration<>(T const&);
    T const& t_;
    ReverseIteration(T const& t) : t_{t} {};
  public:
    auto begin() const { return crbegin(t_); }
    auto end() const { return crend(t_); }
  };

  template <typename T>
  ReverseIteration<T> reverse_iteration(T const& t)
  {
    return ReverseIteration<T>{t};
  }

  auto ptr_for_empty_product_lookup()
  {
    static art::ProductLookup_t::value_type const emptyLookup{};
    return cet::make_exempt_ptr(&emptyLookup);
  }

  auto ptr_for_empty_view_lookup()
  {
    static art::ViewLookup_t::value_type const emptyLookup{};
    return cet::make_exempt_ptr(&emptyLookup);
  }

}

Principal::Principal(ProcessConfiguration const& pc,
                     ProcessHistoryID const& hist,
                     cet::exempt_ptr<PresenceSet const> presentProducts,
                     std::unique_ptr<BranchMapper>&& mapper,
                     std::unique_ptr<DelayedReader>&& reader)
  : processConfiguration_{pc}
  , productLookup_{ptr_for_empty_product_lookup()}
  , viewLookup_{ptr_for_empty_view_lookup()}
  , presentProducts_{presentProducts}
  , branchMapperPtr_{std::move(mapper)}
  , store_{std::move(reader)}
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
Principal::addToProcessHistory()
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
  // OPTIMIZATION NOTE: As of 0_9_0_pre3 For very simple Sources
  // (e.g. EmptyEvent) this routine takes up nearly 50% of the time
  // per event, and 96% of the time for this routine is spent in
  // computing the ProcessHistory id which happens because we are
  // reconstructing the ProcessHistory for each event.  It would
  // probably be better to move the ProcessHistory construction out to
  // somewhere which persists for longer than one Event.
  ProcessHistoryRegistry::emplace(ph.id(), ph);
  setProcessHistoryID(ph.id());
  processHistoryModified_ = true;
}

GroupQueryResult
Principal::getBySelector(WrappedTypeID const& wrapped,
                         SelectorBase const& sel) const
{
  auto const& results = findGroupsForProduct(wrapped, sel, true);
  if (results.empty()) {
    auto whyFailed = std::make_shared<art::Exception>(art::errors::ProductNotFound);
    *whyFailed << "getBySelector: Found zero products matching all criteria\n"
               << "Looking for type: "
               << wrapped.product_type
               << "\n";
    return GroupQueryResult{whyFailed};
  }
  if (results.size() > 1) {
    throw art::Exception(art::errors::ProductNotFound)
      << "getBySelector: Found "
      << results.size()
      << " products rather than one which match all criteria\n"
      << "Looking for type: "
      << wrapped.product_type
      << "\n";
  }
  return results[0];
}

GroupQueryResult
Principal::getByProductID(ProductID const pid) const
{
  if (auto const g = getGroupForPtr(pid)) {
    return GroupQueryResult{g};
  }
  auto whyFailed = std::make_shared<art::Exception>(art::errors::ProductNotFound, "InvalidID");
  *whyFailed << "getGroup: no product with given product id: " << pid << "\n";
  return GroupQueryResult{whyFailed};
}

GroupQueryResult
Principal::getByLabel(WrappedTypeID const& wrapped,
                      string const& label,
                      string const& productInstanceName,
                      string const& processName) const
{
  Selector const sel(ModuleLabelSelector{label} &&
                     ProductInstanceNameSelector{productInstanceName} &&
                     ProcessNameSelector{processName});
  auto const& results = findGroupsForProduct(wrapped, sel, true);
  if (results.empty()) {
    auto whyFailed = std::make_shared<art::Exception>(art::errors::ProductNotFound);
    *whyFailed << "getByLabel: Found zero products matching all criteria\n"
               << "Looking for type: "
               << wrapped.product_type
               << "\n"
               << "Looking for module label: "
               << label
               << "\n"
               << "Looking for productInstanceName: "
               << productInstanceName
               << "\n"
               << (processName.empty() ? "" : "Looking for process: ")
               << processName;
    return GroupQueryResult{whyFailed};
  }
  if (results.size() > 1) {
    throw art::Exception(art::errors::ProductNotFound)
      << "getByLabel: Found "
      << results.size()
      << " products rather than one which match all criteria\n"
      << "Looking for type: "
      << wrapped.product_type
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

Principal::GroupQueryResultVec
Principal::getMany(WrappedTypeID const& wrapped,
                   SelectorBase const& sel) const
{
  return findGroupsForProduct(wrapped, sel, false);
}

int
Principal::tryNextSecondaryFile() const
{
  int const err = store_->openNextSecondaryFile(nextSecondaryFileIdx_);
  if (err != -2) {
    // there are more files to try
    ++nextSecondaryFileIdx_;
  }
  return err;
}

Principal::GroupQueryResultVec
Principal::getMatchingSequence(SelectorBase const& selector) const
{
  GroupQueryResultVec results;

  // Find groups from current process
  for (auto lookup : reverse_iteration(currentProcessViewLookups_)) {
    if (findGroups(*lookup, selector, results, true) != 0) {
      return results;
    }
  }

  // Look through currently opened input files
  if (results.empty()) {
    results = matchingSequenceFromInputFile(selector);
    if (!results.empty()) {
      return results;
    }

    for (auto const& sp : secondaryPrincipals_) {
      results = sp->matchingSequenceFromInputFile(selector);
      if (!results.empty()) {
        return results;
      }
    }
  }

  // Open more secondary files if necessary
  if (results.empty()) {
    while (true) {
      int const err = tryNextSecondaryFile();
      if (err == -2) {
        // No more files.
        break;
      }
      if (err == -1) {
        // Run, SubRun, or Event not found.
        continue;
      }
      assert(!secondaryPrincipals_.empty());
      auto& new_sp = secondaryPrincipals_.back();
      results = new_sp->matchingSequenceFromInputFile(selector);
      if (!results.empty()) {
        return results;
      }
    }
  }

  return results;
}

Principal::GroupQueryResultVec
Principal::matchingSequenceFromInputFile(SelectorBase const& selector) const
{
  assert(viewLookup_);
  GroupQueryResultVec results;
  findGroups(*viewLookup_, selector, results, true);
  return results;
}

void
Principal::removeCachedProduct(ProductID const pid) const
{
  if (auto g = getGroup(pid)) {
    g->removeCachedProduct();
    return;
  }
  for (auto const& sp : secondaryPrincipals_) {
    if (auto g = sp->getGroup(pid)) {
      g->removeCachedProduct();
      return;
    }
  }
  throw Exception(errors::ProductNotFound, "removeCachedProduct")
    << "Attempt to remove unknown product corresponding to ProductID: " << pid << '\n'
    << "Please contact artists@fnal.gov\n";
}

Principal::GroupQueryResultVec
Principal::findGroupsForProduct(WrappedTypeID const& wrapped,
                                SelectorBase const& selector,
                                bool const stopIfProcessHasMatch) const
{
  GroupQueryResultVec results;

  unsigned ret{};
  // Find groups from current process
  for (auto lookup : reverse_iteration(currentProcessProductLookups_)) {
    auto it = lookup->find(wrapped.product_type.friendlyClassName());
    if (it == lookup->end()) {
      continue;
    }
    ret += findGroups(it->second, selector, results, stopIfProcessHasMatch, wrapped.wrapped_product_type);
  }

  // Look through currently opened input files
  ret += findGroupsFromInputFile(wrapped, selector, results, stopIfProcessHasMatch);
  if (ret) {
    return results;
  }

  for (auto const& sp : secondaryPrincipals_) {
    if (sp->findGroupsFromInputFile(wrapped, selector, results, stopIfProcessHasMatch)) {
      return results;
    }
  }

  // Open more secondary files if necessary
  while (true) {
    int const err = tryNextSecondaryFile();
    if (err == -2) {
      // No more files.
      break;
    }
    if (err == -1) {
      // Run, SubRun, or Event not found.
      continue;
    }
    assert(!secondaryPrincipals_.empty());
    auto& new_sp = secondaryPrincipals_.back();
    if (new_sp->findGroupsFromInputFile(wrapped, selector, results, stopIfProcessHasMatch)) {
      return results;
    }
  }
  return results;
}

std::size_t
Principal::findGroupsFromInputFile(WrappedTypeID const& wrapped,
                                   SelectorBase const& selector,
                                   GroupQueryResultVec& results,
                                   bool const stopIfProcessHasMatch) const
{
  assert(productLookup_);
  auto it = productLookup_->find(wrapped.product_type.friendlyClassName());
  if (it == productLookup_->end()) {
    return 0;
  }
  return findGroups(it->second, selector, results, stopIfProcessHasMatch, wrapped.wrapped_product_type);
}

std::size_t
Principal::findGroups(ProcessLookup const& pl,
                      SelectorBase const& sel,
                      GroupQueryResultVec& res,
                      bool const stopIfProcessHasMatch,
                      TypeID const wanted_wrapper/*=TypeID()*/) const
{
  // Loop over processes in reverse time order.  Sometimes we want to
  // stop after we find a process with matches so check for that at
  // each step.
  std::size_t found{};
  for (auto const& h : reverse_iteration(processHistory())) {
    auto it = pl.find(h.processName());
    if (it != pl.end()) {
      found += findGroupsForProcess(it->second, sel, res, wanted_wrapper);
    }
    if (stopIfProcessHasMatch && !res.empty())  break;
  }
  return found;
}

std::size_t
Principal::findGroupsForProcess(std::vector<ProductID> const& vpid,
                                SelectorBase const& sel,
                                GroupQueryResultVec& res,
                                TypeID const wanted_wrapper) const
{
  std::size_t found{}; // Horrible hack that should go away
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
    ++found;
  }
  return found;
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
Principal::getForOutput(ProductID const pid, bool const resolveProd) const
{
  auto const& g = getResolvedGroup(pid, resolveProd);
  if (g.get() == nullptr) {
    return OutputHandle{RangeSet::invalid()};
  }

  auto const& pd = g->productDescription();
  if (resolveProd
      &&
      ((g->anyProduct() == nullptr) || !g->anyProduct()->isPresent())
      &&
      (presentFromSource(pid) || pd.produced())
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

cet::exempt_ptr<Group const>
Principal::getResolvedGroup(ProductID const pid,
                            bool const resolveProd) const
{
  // FIXME: This reproduces the behavior of the original getGroup with
  // resolveProv == false but I am not sure this is correct in the
  // case of an unavailable product.
  auto const g = getGroupForPtr(pid);
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

bool
Principal::presentFromSource(ProductID const pid) const
{
  bool present{false};
  if (presentProducts_) {
    present = (presentProducts_->find(pid) != presentProducts_->cend());
  }
  return present;
}

cet::exempt_ptr<Group const>
Principal::getGroupForPtr(ProductID const pid) const
{
  bool produced{false};
  for (auto prod : reverse_iteration(currentProcessProducedProducts_)) {
    if (prod->find(pid) != prod->cend()) {
      produced = true;
      break;
    }
  }

  // Look through current process and currently opened primary input file.
  if (produced || presentFromSource(pid)) {
    return getGroup(pid);
  }

  // Look through secondary files
  for (auto const& sp : secondaryPrincipals_) {
    if (sp->presentFromSource(pid)) {
      return sp->getGroup(pid);
    }
  }

  // Try new secondary files
  while (true) {
    int const err = tryNextSecondaryFile();
    if (err == -2) {
      // No more files.
      return nullptr;
    }
    if (err == -1) {
      // Run, SubRun, or Event not found.
      continue;
    }
    assert(!secondaryPrincipals_.empty());
    auto& new_sp = secondaryPrincipals_.back();
    if (new_sp->presentFromSource(pid)) {
      return new_sp->getGroup(pid);
    }
  }

  return nullptr;
}

cet::exempt_ptr<Group const>
Principal::getGroup(ProductID const pid) const
{
  auto it = groups_.find(pid);
  return it != groups_.cend() ? it->second.get() : nullptr;
}

EDProductGetter const*
Principal::productGetter(ProductID const pid) const
{
  EDProductGetter const* result{getByProductID(pid).result().get()};
  return result ? result : deferredGetter_(pid);
}
