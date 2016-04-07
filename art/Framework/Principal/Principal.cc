#include "art/Framework/Principal/Principal.h"
// vim: set sw=2:

#include "art/Framework/Principal/Selector.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
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
~Principal()
{
  primaryPrincipal_ = nullptr;
}

Principal::
Principal(ProcessConfiguration const& pc, ProcessHistoryID const& hist,
          std::unique_ptr<BranchMapper>&& mapper,
          std::unique_ptr<DelayedReader>&& reader, int idx,
          Principal* primaryPrincipal)
  : processHistoryPtr_(new ProcessHistory)
  , processConfiguration_(pc)
  , processHistoryModified_(false)
  , groups_()
  , branchMapperPtr_(std::move(mapper))
  , store_(std::move(reader))
  , primaryPrincipal_(primaryPrincipal)
  , secondaryPrincipals_()
  , secondaryIdx_(idx)
  , nextSecondaryFileIdx_(0)
{
  if (!hist.isValid()) {
    return;
  }
  assert(!ProcessHistoryRegistry::empty());
  bool found [[gnu::unused]] = ProcessHistoryRegistry::get(hist, *processHistoryPtr_);
  assert(found);
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
  // OPTIMIZATION NOTE:  As of 0_9_0_pre3
  // For very simple Sources (e.g. EmptyEvent) this routine takes
  // up nearly 50% of the time per event, and 96% of the time for
  // this routine is spent in computing the ProcessHistory id which
  // happens because we are reconstructing the ProcessHistory for
  // each event (the process ID is first computed in the call to
  // insertMapped() below).  It would probably be better to move
  // the ProcessHistory construction out to somewhere which persists
  // for longer than one Event.
  ProcessHistoryRegistry::put(ph);
  setProcessHistoryID(ph.id());
  processHistoryModified_ = true;
}

GroupQueryResult
Principal::
getBySelector(TypeID const& productType, SelectorBase const& sel) const
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
Principal::
getByLabel(TypeID const& productType, string const& label,
           string const& productInstanceName, string const& processName) const
{
  GroupQueryResultVec results;
  Selector sel(ModuleLabelSelector(label) &&
               ProductInstanceNameSelector(productInstanceName) &&
               ProcessNameSelector(processName));
  int nFound = findGroupsForProduct(productType, sel, results, true);
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

void
Principal::
getManyByType(TypeID const& productType, GroupQueryResultVec& results) const
{
  MatchAllSelector sel;
  findGroupsForProduct(productType, sel, results, false);
}

int
Principal::
tryNextSecondaryFile() const
{
  int err = store_->openNextSecondaryFile(nextSecondaryFileIdx_);
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
  // Handle groups for current process, note that we need to
  // look at the current process even if it is not in the processHistory
  // because of potential unscheduled (onDemand) production
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
findGroupsForProcess(std::vector<BranchID> const& vbid,
                     SelectorBase const& sel,
                     GroupQueryResultVec& res,
                     TypeID wanted_wrapper) const
{

  for (auto const bid : vbid) {
    auto group = getGroup(bid);
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
      group->resolveProduct(true, wanted_wrapper);
    }
    else {
      group->resolveProduct(true, group->producedWrapperType());
    }
    // If the product is a dummy filler, group will now be marked unavailable.
    // Unscheduled execution can fail to produce the EDProduct so check.
    if (!group->productUnavailable() && !group->onDemand()) {
      // Found a good match, save it.
      res.push_back(GroupQueryResult(group.get()));
    }
  }
}

OutputHandle
Principal::
getForOutput(BranchID const bid, bool resolveProd) const
{
  auto const& g = getResolvedGroup(bid, resolveProd, false);
  if (!g) {
    return OutputHandle{};
  }
  auto const & pmd = ProductMetaData::instance();
  auto const bt = g->productDescription().branchType();
  if (resolveProd
      &&
      ((g->anyProduct() == nullptr) || !g->anyProduct()->isPresent())
      &&
      ( pmd.presentWithFileIdx(bt, bid) != MasterProductRegistry::DROPPED ||
        pmd.produced(bt, bid) )
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
    return OutputHandle{};
  }
  return OutputHandle{g->anyProduct(), &g->productDescription(), g->productProvenancePtr(), g->rangeSetIDIsSet()};
}

std::shared_ptr<const Group> const
Principal::
getResolvedGroup(BranchID const& bid,
                 bool resolveProd,
                 bool fillOnDemand) const
{
  // FIXME: This reproduces the behavior of the original getGroup with
  // resolveProv == false but I am not sure this is correct in the case
  // of an unavailable product.
  std::shared_ptr<const Group> const& g(getGroupForPtr(branchType(),bid));
  if (!g.get() || !resolveProd) {
    return g;
  }
  bool gotIt = g->resolveProductIfAvailable(fillOnDemand,
                                            g->producedWrapperType());
  if (!gotIt && g->onDemand()) {
    // Behavior is the same as if the group wasn't there.
    return nullptr;
  }
  return g;
}

cet::exempt_ptr<Group const>
Principal::
getExistingGroup(BranchID const& bid) const
{
  auto I = groups_.find(bid);
  if (I != groups_.end()) {
    return I->second.get();
  }
  for (auto p : secondaryPrincipals_) {
    auto I = p->groups_.find(bid);
    if (I != p->groups_.end()) {
      return I->second.get();
    }
  }
  return nullptr;
}

std::shared_ptr<const Group> const
Principal::
getGroupForPtr(BranchType const btype, BranchID const bid) const
{
  const Principal* pp = this;
  if (primaryPrincipal_ != nullptr) {
    pp = primaryPrincipal_.get();
  }

  std::size_t const index    = ProductMetaData::instance().presentWithFileIdx(btype, bid);
  bool        const produced = ProductMetaData::instance().produced(btype, bid);
  if ( produced || index == 0 ) {
    auto it = pp->groups_.find(bid);
    // Note: There will be groups for dropped products, so we
    //       must check for that.  We want the group where the
    //       product can actually be retrieved from.
    return (it != pp->groups_.end()) ? it->second : nullptr;
  }
  else if ( index > 0 && ( index-1 < secondaryPrincipals_.size() ) ) {
    auto const& groups = secondaryPrincipals_[index-1]->groups_;
    auto it = groups.find(bid);
    return (it != groups.end()) ? it->second : nullptr;
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
    std::size_t const index = ProductMetaData::instance().presentWithFileIdx(btype, bid);
    if ( index == MasterProductRegistry::DROPPED ) continue;
    auto p = secondaryPrincipals_[index-1];
    auto it = p->groups_.find(bid);
    // Note: There will be groups for dropped products, so we
    //       must check for that.  We want the group where the
    //       product can actually be retrieved from.
    return (it != p->groups_.end()) ? it->second : nullptr;
  }
}

std::shared_ptr<const Group> const
Principal::
getGroup(BranchID const& bid) const
{
  const Principal* pp = this;
  if (primaryPrincipal_ != nullptr) {
    pp = primaryPrincipal_.get();
  }
  {
    auto I = pp->groups_.find(bid);
    if (I != pp->groups_.end()) {
      return I->second;
    }
  }
  for (auto p : secondaryPrincipals_) {
    auto I = p->groups_.find(bid);
    if (I != p->groups_.end()) {
      return I->second;
    }
  }
  return nullptr;
}

} // namespace art
