#include "art/Framework/Principal/Principal.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/DelayedReader.h"
#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/OutputHandle.h"
#include "art/Framework/Principal/ProcessTag.h"
#include "art/Framework/Principal/RangeSetsSupported.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Common/WrappedTypeID.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/Parentage.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/fwd.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "range/v3/view.hpp"

#include <atomic>
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace cet;
using namespace std;

namespace {
  std::string const indent(2, ' ');
}

namespace art {

  namespace {

    unique_ptr<Group>
    create_group(DelayedReader* reader, BranchDescription const& bd)
    {
      auto const& class_name = bd.producedClassName();
      auto gt = Group::grouptype::normal;
      if (is_assns(class_name)) {
        if (name_of_template_arg(class_name, 2) == "void"s) {
          gt = Group::grouptype::assns;
        } else {
          gt = Group::grouptype::assnsWithData;
        }
      }
      return make_unique<Group>(
        reader, bd, make_unique<RangeSet>(RangeSet::invalid()), gt);
    }

  } // unnamed namespace

  void
  Principal::ctor_create_groups(
    cet::exempt_ptr<ProductTable const> presentProducts)
  {
    if (!presentProducts) {
      return;
    }
    // Note: Dropped products are a problem. We should not create
    //       groups for them now because later we may open a secondary
    //       file which actually contains them and we want the
    //       secondary principal to have those groups. However some
    //       code expects to be able to find a group for dropped
    //       products, so getGroupTryAllFiles ignores groups for
    //       dropped products instead.
    for (auto const& pd :
         presentProducts->descriptions | ranges::views::values) {
      assert(pd.branchType() == branchType_);
      fillGroup(pd);
    }
  }

  void
  Principal::ctor_read_provenance()
  {
    auto ppv = delayedReader_->readProvenance();
    for (auto iter = ppv.begin(), end = ppv.end(); iter != end; ++iter) {
      auto g = getGroupLocal(iter->productID());
      if (g.get() == nullptr) {
        continue;
      }
      if (iter->productStatus() != productstatus::unknown()) {
        g->setProductProvenance(make_unique<ProductProvenance>(*iter));
      } else {
        // We have an old format file, convert.
        g->setProductProvenance(make_unique<ProductProvenance>(
          iter->productID(),
          productstatus::dummyToPreventDoubleCount(),
          iter->parentage().parents()));
      }
    }
  }

  void
  Principal::ctor_fetch_process_history(ProcessHistoryID const& phid)
  {
    if (!phid.isValid()) {
      return;
    }
    ProcessHistory processHistory;
    ProcessHistoryRegistry::get(phid, processHistory);
    std::swap(processHistory_, processHistory);
  }

  Principal::Principal(BranchType branchType,
                       ProcessConfiguration const& pc,
                       cet::exempt_ptr<ProductTable const> presentProducts,
                       ProcessHistoryID const& hist,
                       std::unique_ptr<DelayedReader>&&
                         reader /* = std::make_unique<NoDelayedReader>() */)
    : branchType_{branchType}
    , processConfiguration_{pc}
    , presentProducts_{presentProducts.get()}
    , delayedReader_{move(reader)}
  {
    delayedReader_->setPrincipal(this);
    ctor_create_groups(presentProducts);
    ctor_read_provenance();
    ctor_fetch_process_history(hist);
  }

  void
  Principal::fillGroup(BranchDescription const& pd)
  {
    std::lock_guard sentry{groupMutex_};
    auto it = groups_.find(pd.productID());
    if (it != std::cend(groups_)) {
      // The 'combinable' call does not require that the processing
      // history be the same, which is not what we are checking for here.
      auto const& found_pd = it->second->productDescription();
      if (combinable(found_pd, pd)) {
        throw Exception(errors::Configuration)
          << "The process name " << pd.processName()
          << " was previously used on these products.\n"
          << "Please modify the configuration file to use a "
          << "distinct process name.\n";
      }
      throw Exception(errors::ProductRegistrationFailure)
        << "The product ID " << pd.productID() << " of the new product:\n"
        << pd
        << " collides with the product ID of the already-existing product:\n"
        << found_pd
        << "Please modify the instance name of the new product so as to avoid "
           "the product ID collision.\n"
        << "In addition, please notify artists@fnal.gov of this error.\n";
    }

    groups_[pd.productID()] = create_group(delayedReader_.get(), pd);
  }

  // FIXME: This breaks the purpose of the
  //        Principal::addToProcessHistory() compare_exchange_strong
  //        because of the temporal hole between when the history is
  //        changed and when the flag is set, this must be fixed!
  void
  Principal::markProcessHistoryAsModified()
  {
    processHistoryModified_ = true;
  }

  // Note: LArSoft uses this extensively to create a Ptr by hand.
  EDProductGetter const*
  Principal::productGetter(ProductID const& pid) const
  {
    auto g = getGroupTryAllFiles(pid);
    if (g.get() != nullptr) {
      // Note: All produced products should be found.
      return g.get();
    }
    return nullptr;
  }

  EDProductGetter const*
  Principal::getEDProductGetter_(ProductID const& pid) const
  {
    return productGetter(pid);
  }

  void
  Principal::createGroupsForProducedProducts(
    ProductTables const& producedProducts)
  {
    auto const& produced = producedProducts.get(branchType_);
    producedProducts_ = &produced;
    if (produced.descriptions.empty()) {
      return;
    }
    // The process history is expanded if there is a product that is
    // produced in this process.
    addToProcessHistory();
    for (auto const& pd : produced.descriptions | ranges::views::values) {
      assert(pd.branchType() == branchType_);
      // Create a group for the produced product.
      fillGroup(pd);
    }
  }

  void
  Principal::enableLookupOfProducedProducts()
  {
    enableLookupOfProducedProducts_ = true;
  }

  void
  Principal::readImmediate() const
  {
    // Read all data products and provenance immediately, if
    // available.  Used only by RootInputFile to implement the
    // delayedRead*Products config options.
    //
    // Note: The input source lock will be held when this routine is called.
    //
    // MT-TODO: For right now ignore the delay reading option for
    //          product provenance. If we do the delay reading then we
    //          must use a lock to interlock all fetches of provenance
    //          because the delay read fills the pp_by_pid_ one entry
    //          at a time, and we do not want other threads to find
    //          the info only partly there.
    std::lock_guard sentry{groupMutex_};
    for (auto const& pid_and_group : groups_) {
      auto group = pid_and_group.second.get();
      group->resolveProductIfAvailable();
    }
  }

  ProcessHistory const&
  Principal::processHistory() const
  {
    // MT note: We make no attempt to protect callers who use this
    //          call to get access to the iteration interface of the
    //          process history.  See the threading notes there and
    //          here for the reasons why.
    return processHistory_;
  }

  ProcessConfiguration const&
  Principal::processConfiguration() const
  {
    return processConfiguration_;
  }

  size_t
  Principal::size() const
  {
    std::lock_guard sentry{groupMutex_};
    return groups_.size();
  }

  Principal::const_iterator
  Principal::begin() const
  {
    std::lock_guard sentry{groupMutex_};
    return groups_.begin();
  }

  Principal::const_iterator
  Principal::cbegin() const
  {
    std::lock_guard sentry{groupMutex_};
    return groups_.cbegin();
  }

  Principal::const_iterator
  Principal::end() const
  {
    std::lock_guard sentry{groupMutex_};
    return groups_.end();
  }

  Principal::const_iterator
  Principal::cend() const
  {
    std::lock_guard sentry{groupMutex_};
    return groups_.cend();
  }

  cet::exempt_ptr<ProductProvenance const>
  Principal::branchToProductProvenance(ProductID const& pid) const
  {
    // Note: The input source lock will be held when this routine is called.
    //
    // MT-TODO: For right now ignore the delay reading option for
    //          product provenance. If we do the delay reading then we
    //          must use a lock to interlock all fetches of provenance
    //          because the delay read fills the pp_by_pid_ one entry
    //          at a time, and we do not want other threads to find
    //          the info only partly there.
    cet::exempt_ptr<ProductProvenance const> ret;
    auto g = getGroupLocal(pid);
    if (g.get() != nullptr) {
      ret = g->productProvenance();
    }
    return ret;
  }

  // Note: threading: The solution chosen for the following
  // problems is to convert groups_ from type:
  //
  //   std::map<ProductID, std::unique_ptr<Group>>
  //
  // to type:
  //
  //   tbb::concurrent_unordered_map<
  //     ProductID, std::unique_ptr<Group>>
  //
  // We get concurrent insertion and iteration, but not
  // concurrent erasure (which is not a problem because we never
  // remove groups).  Note that tbb uses a value for the end()
  // iterator for this class which is always valid for comparing
  // against the result of an interation or find (it is
  // implemented as (<element-ptr>(nullptr), <internal-table>)).
  //
  // Note: threading: May be called from producer and filter
  // module processing tasks! This requires us to protect
  // groups_ against multiple threads attempting an insert at
  // the same time.
  //
  // Note: threading: Also anyone using the iterators over
  // groups_ (which are Principal::begin() and Principal::end())
  // need protection against having their interators invalidated.
  // Right now the only code doing this is:
  //
  //   Principal::readImmediate() const
  //   Principal::getGroupTryAllFiles(ProductID const& pid) const
  //   Principal::removeCachedProduct(ProductID const pid) const
  //   Principal::findGroupsForProcess(...) const
  //   OutputModule::updateBranchChildren()
  //
  // Principal::readImmediate() is called just after principal
  // creation with the input lock held.  Module tasks on other
  // streams could be doing puts which would invalid the iterator
  // if the data product being put does not exist in the input
  // file and is being put for the first time so this is a
  // group insertion.
  //
  // Principal::getGroupTryAllFiles(ProductID const& pid) const
  //   Used by Principal::getByProductID(ProductID const& pid) const
  //     Used by art::ProductRetriever<T>::get(ProductID const pid, Handle<T>&
  //       result) const. (easy user-facing api)
  //     Used by Principal::productGetter(ProductID const pid) const
  //       Used by (Run,SubRun,Event,Results)::productGetter (advanced
  //         user-facing api)
  //   Used by Principal::getForOutput(ProductID const pid, bool
  //       resolveProd) const
  //     Used by RootOutputFile to fetch products being written to disk.
  //     Used by FileDumperOutput_module.
  //     Used by ProvenanceCheckerOutput_module.
  //
  // These uses are find() and compare against end().  Problem is that
  // end() may have moved by the time we do the compare with the find
  // result. There is also use of the mpr and secondary files.
  //
  // Principal::removeCachedProduct(ProductID const pid) const
  // Principal::findGroupsForProcess(...) const
  //
  // These uses are find() and compare against end().  Problem is that
  // end() may have moved by the time we do the compare with the find
  // result. There is also use of secondary principals.
  //
  // OutputModule::updateBranchChildren is called only for events
  // and only after all module processing tasks which can put
  // products into the event have finished running, so it does
  // not need the protection.
  //
  // Used by the Run, SubRun, and EventPrincipal constructors
  // if a product was produced.
  //
  // Used by RootOutput_module from write, writeSubRun, and
  // writeRun if a branch was dropped by selectEvents processing
  // for that kind of principal.
  //
  // Used by RootOutput_module from startEndFile if a branch was
  // dropped by selectEvents processing for a results principal,
  // or if a product was produced by a results principal (note
  // that it has not actually gotten the chance to make the
  // product, that happens right after this call is made).
  //
  // Note: threading: If the only uses were from the constructors
  // we would have no problems, but the use from the root output
  // module is bad because it could be running concurrently with
  // other output modules and analyzers for this same principal.
  // So we have to use a compare_exchange_strong on
  // processHistoryModified_ so that only one task tries to do
  // this. We also need to stall the other output and analyzer
  // modules that call processHistory() while we are doing the
  // update so that they get the updated result.  For output and
  // analyzer modules that have already fetched the process
  // history pointer, we have to stall any attempt to access its
  // internal process configuration list while we are updating it.
  //
  // FIXME: threading: We hand out processHistory_ through the
  // processHistory() interface, which is in turn handed out by
  // the ProductRetriever::processHistory() interface to any module
  // task that wants it.  This is a problem for output modules
  // and analyzers if an output module decides to update the
  // process history from startEndFile. We must stall users of
  // the process history if we updating it, both by stalling a
  // fetch of processHistory_ once we start to update, and for
  // those modules that have already fetched the processHistory_
  // we must stall any attempt by them to access its internal
  // process configuration list while we are changing it.
  void
  Principal::addToProcessHistory()
  {
    bool expected = false;
    if (processHistoryModified_.compare_exchange_strong(expected, true)) {
      // MT note: We have now locked out any other task trying to
      //          modify the process history. Now we have to block
      //          tasks that already have a pointer to the process
      //          history from accessing its internals while we update
      //          it. We do not protect the iteration interface, the
      //          begin(), end(), and size() are all separate calls
      //          and we cannot lock in each one because there is no
      //          way to automatically unlock.
      std::lock_guard sentry{processHistory_.get_mutex()};
      string const& processName = processConfiguration_.processName();
      for (auto const& val : processHistory_) {
        if (processName == val.processName()) {
          throw art::Exception(errors::Configuration)
            << "The process name " << processName
            << " was previously used on these products.\n"
            << "Please modify the configuration file to use a "
            << "distinct process name.\n";
        }
      }
      processHistory_.push_back(processConfiguration_);
      // Optimization note: As of 0_9_0_pre3 For very simple Sources
      // (e.g. EmptyEvent) this routine takes up nearly 50% of the
      // time per event, and 96% of the time for this routine is spent
      // in computing the ProcessHistory id which happens because we
      // are reconstructing the ProcessHistory for each event.  It
      // would probably be better to move the ProcessHistory
      // construction out to somewhere which persists for longer than
      // one Event.
      auto const phid = processHistory_.id();
      ProcessHistoryRegistry::emplace(phid, processHistory_);
    }
  }

  std::size_t
  Principal::findGroups(ProcessLookup const& pl,
                        ModuleContext const& mc,
                        SelectorBase const& sel,
                        std::vector<cet::exempt_ptr<Group>>& groups) const
  {
    // Loop over processes in reverse time order.  Sometimes we want
    // to stop after we find a process with matches so check for that
    // at each step.
    std::size_t found{};
    // MT note: We must protect the process history iterators here
    //          against possible invalidation by output modules
    //          inserting a process history entry while we are
    //          iterating.
    std::lock_guard sentry{processHistory_.get_mutex()};
    // We must skip over duplicate entries of the same process
    // configuration in the process history.  This unfortunately
    // happened with the SamplingInput source.
    for (auto const& h :
         ranges::views::reverse(processHistory_) | ranges::views::unique) {
      if (auto it = pl.find(h.processName()); it != pl.end()) {
        found += findGroupsForProcess(it->second, mc, sel, groups);
      }
    }
    return found;
  }

  GroupQueryResult
  Principal::getBySelector(ModuleContext const& mc,
                           WrappedTypeID const& wrapped,
                           SelectorBase const& sel,
                           ProcessTag const& processTag) const
  {
    auto const groups = findGroupsForProduct(mc, wrapped, sel, processTag);
    auto const result = resolve_unique_product(groups, wrapped);
    if (!result.has_value()) {
      auto whyFailed = std::make_shared<Exception>(errors::ProductNotFound);
      *whyFailed << "Found zero products matching all selection criteria\n"
                 << indent << "C++ type: " << wrapped.product_type << '\n'
                 << sel.print(indent) << '\n';
      return GroupQueryResult{whyFailed};
    }
    return *result;
  }

  GroupQueryResult
  Principal::getByLabel(ModuleContext const& mc,
                        WrappedTypeID const& wrapped,
                        string const& label,
                        string const& productInstanceName,
                        ProcessTag const& processTag) const
  {
    auto const& processName = processTag.name();
    Selector const sel{ModuleLabelSelector{label} &&
                       ProductInstanceNameSelector{productInstanceName} &&
                       ProcessNameSelector{processName}};
    return getBySelector(mc, wrapped, sel, processTag);
  }

  std::vector<InputTag>
  Principal::getInputTags(ModuleContext const& mc,
                          WrappedTypeID const& wrapped,
                          SelectorBase const& sel,
                          ProcessTag const& processTag) const
  {
    std::vector<InputTag> tags;
    auto const groups = findGroupsForProduct(mc, wrapped, sel, processTag);
    cet::transform_all(groups, back_inserter(tags), [](auto const g) {
      return g->productDescription().inputTag();
    });
    return tags;
  }

  std::vector<GroupQueryResult>
  Principal::getMany(ModuleContext const& mc,
                     WrappedTypeID const& wrapped,
                     SelectorBase const& sel,
                     ProcessTag const& processTag) const
  {
    auto const groups = findGroupsForProduct(mc, wrapped, sel, processTag);
    return resolve_products(groups, wrapped.wrapped_product_type);
  }

  auto
  Principal::tryNextSecondaryFile() const
  {
    return delayedReader_->readFromSecondaryFile(nextSecondaryFileIdx_);
  }

  std::vector<cet::exempt_ptr<Group>>
  Principal::getMatchingSequence(ModuleContext const& mc,
                                 SelectorBase const& selector,
                                 ProcessTag const& processTag) const
  {
    std::vector<cet::exempt_ptr<Group>> groups;
    // Find groups from current process
    if (processTag.current_process_search_allowed() &&
        enableLookupOfProducedProducts_.load()) {
      if (findGroups(
            producedProducts_.load()->viewLookup, mc, selector, groups) != 0) {
        return groups;
      }
    }

    if (!processTag.input_source_search_allowed()) {
      return groups;
    }

    // Look through currently opened input files
    if (groups.empty()) {
      groups = matchingSequenceFromInputFile(mc, selector);
      if (!groups.empty()) {
        return groups;
      }
      for (auto const& sp : secondaryPrincipals_) {
        groups = sp->matchingSequenceFromInputFile(mc, selector);
        if (!groups.empty()) {
          return groups;
        }
      }
    }
    // Open more secondary files if necessary
    if (groups.empty()) {
      while (auto sp = tryNextSecondaryFile()) {
        auto& new_sp = secondaryPrincipals_.emplace_back(move(sp));
        groups = new_sp->matchingSequenceFromInputFile(mc, selector);
        if (!groups.empty()) {
          return groups;
        }
      }
    }
    return groups;
  }

  std::vector<cet::exempt_ptr<Group>>
  Principal::matchingSequenceFromInputFile(ModuleContext const& mc,
                                           SelectorBase const& selector) const
  {
    std::vector<cet::exempt_ptr<Group>> groups;
    if (!presentProducts_.load()) {
      return groups;
    }
    findGroups(presentProducts_.load()->viewLookup, mc, selector, groups);
    return groups;
  }

  std::size_t
  Principal::findGroupsFromInputFile(
    ModuleContext const& mc,
    WrappedTypeID const& wrapped,
    SelectorBase const& selector,
    std::vector<cet::exempt_ptr<Group>>& groups) const
  {
    if (!presentProducts_.load()) {
      return 0;
    }
    auto const& lookup = presentProducts_.load()->productLookup;
    auto it = lookup.find(wrapped.product_type.friendlyClassName());
    if (it == lookup.end()) {
      return 0;
    }
    return findGroups(it->second, mc, selector, groups);
  }

  std::vector<cet::exempt_ptr<Group>>
  Principal::findGroupsForProduct(ModuleContext const& mc,
                                  WrappedTypeID const& wrapped,
                                  SelectorBase const& selector,
                                  ProcessTag const& processTag) const
  {
    std::vector<cet::exempt_ptr<Group>> results;
    unsigned ret{};
    // Find groups from current process
    if (processTag.current_process_search_allowed() &&
        enableLookupOfProducedProducts_.load()) {
      auto const& lookup = producedProducts_.load()->productLookup;
      auto it = lookup.find(wrapped.product_type.friendlyClassName());
      if (it != lookup.end()) {
        ret += findGroups(it->second, mc, selector, results);
      }
    }

    if (!processTag.input_source_search_allowed()) {
      return results;
    }

    // Look through currently opened input files
    ret += findGroupsFromInputFile(mc, wrapped, selector, results);
    if (ret) {
      return results;
    }
    for (auto const& sp : secondaryPrincipals_) {
      if (sp->findGroupsFromInputFile(mc, wrapped, selector, results)) {
        return results;
      }
    }
    // Open more secondary files if necessary
    while (auto sp = tryNextSecondaryFile()) {
      auto& new_sp = secondaryPrincipals_.emplace_back(move(sp));
      if (new_sp->findGroupsFromInputFile(mc, wrapped, selector, results)) {
        return results;
      }
    }
    return results;
  }

  std::size_t
  Principal::findGroupsForProcess(
    std::vector<ProductID> const& vpid,
    ModuleContext const& mc,
    SelectorBase const& sel,
    std::vector<cet::exempt_ptr<Group>>& res) const
  {
    std::size_t found{}; // Horrible hack that should go away
    for (auto const pid : vpid) {
      auto group = getGroupLocal(pid);
      if (!group) {
        continue;
      }
      auto const& pd = group->productDescription();
      // If we are processing a trigger path, the only visible
      // produced products are those that originate from modules on
      // the same path we're currently processing.
      if (mc.onTriggerPath() && pd.produced() &&
          !mc.onSamePathAs(pd.moduleLabel())) {
        continue;
      }
      if (!sel.match(pd)) {
        continue;
      }
      // Found a good match, save it.
      res.emplace_back(group);
      ++found;
    }
    return found;
  }

  RangeSet
  Principal::seenRanges() const
  {
    return rangeSet_;
  }

  void
  Principal::updateSeenRanges(RangeSet const& rs)
  {
    rangeSet_ = rs;
  }

  void
  Principal::put(BranchDescription const& bd,
                 unique_ptr<ProductProvenance const>&& pp,
                 unique_ptr<EDProduct>&& edp,
                 unique_ptr<RangeSet>&& rs)
  {
    assert(edp);
    if (detail::range_sets_supported(branchType_)) {
      // Note: We intentionally allow group and provenance replacement
      //       for run and subrun products.
      auto group = getGroupLocal(bd.productID());
      assert(group);
      group->setProductAndProvenance(move(pp), move(edp), move(rs));
    } else {
      auto group = getGroupLocal(bd.productID());
      assert(group);
      if (group->anyProduct() != nullptr) {
        throw art::Exception(art::errors::ProductRegistrationFailure,
                             "Principal::put:")
          << "Problem found during put of " << branchType_
          << " product: product already put for " << bd.branchName() << '\n';
      }
      group->setProductAndProvenance(
        move(pp), move(edp), make_unique<RangeSet>(RangeSet::invalid()));
    }
  }

  // We invoke the delay reader now if no user module has ever fetched them
  // for this principal if resolvedProd is true.
  //
  // Note: This attempts to resolve the product and converts the
  //       resulting group into an OutputHandle.
  //
  // MT note: Right now this is single-threaded.  Be careful if this
  //          changes!!!
  OutputHandle
  Principal::getForOutput(ProductID const& pid, bool const resolveProd) const
  {
    // MT-FIXME: Uses of group!
    auto g = getGroupTryAllFiles(pid);
    if (g.get() == nullptr) {
      return OutputHandle::invalid();
    }
    if (resolveProd) {
      if (!g->resolveProductIfAvailable()) {
        // Behavior is the same as if the group wasn't there.
        return OutputHandle::invalid();
      }
      if (g->anyProduct() == nullptr) {
        return OutputHandle::invalid();
      }
      if (!g->anyProduct()->isPresent()) {
        return OutputHandle::invalid();
      }
    }
    if (!g->anyProduct() && !g->productProvenance()) {
      return OutputHandle{g->rangeOfValidity()};
    }
    return OutputHandle{g->anyProduct(),
                        &g->productDescription(),
                        g->productProvenance(),
                        g->rangeOfValidity()};
  }

  cet::exempt_ptr<BranchDescription const>
  Principal::getProductDescription(
    ProductID const pid,
    bool const alwaysEnableLookupOfProducedProducts /*=false*/) const
  {
    // Find groups from current process
    if (alwaysEnableLookupOfProducedProducts ||
        enableLookupOfProducedProducts_.load()) {
      if (producedProducts_.load() != nullptr) {
        if (auto result = producedProducts_.load()->description(pid)) {
          return result;
        }
      }
    }
    if (presentProducts_.load()) {
      // Look through currently opened input files
      if (auto result = presentProducts_.load()->description(pid)) {
        return result;
      }
    }
    for (auto const& sp : secondaryPrincipals_) {
      if (auto result = sp->getProductDescription(pid)) {
        return result;
      }
    }
    return nullptr;
  }

  BranchType
  Principal::branchType() const
  {
    return branchType_;
  }

  std::optional<ProductInserter>
  Principal::makeInserter(ModuleContext const& mc)
  {
    return std::make_optional<ProductInserter>(branchType_, *this, mc);
  }

  bool
  Principal::producedInProcess(ProductID const pid) const
  {
    if (!enableLookupOfProducedProducts_.load()) {
      return false;
    }
    auto pd = producedProducts_.load()->description(pid);
    return pd == nullptr ? false : pd->produced();
  }

  bool
  Principal::presentFromSource(ProductID const pid) const
  {
    if (!presentProducts_.load()) {
      return false;
    }
    auto pd = presentProducts_.load()->description(pid);
    return pd == nullptr ? false : pd->present();
  }

  GroupQueryResult
  Principal::getByProductID(ProductID const pid) const
  {
    if (auto g = getGroupTryAllFiles(pid)) {
      return GroupQueryResult{g};
    }
    auto whyFailed =
      make_shared<Exception>(errors::ProductNotFound, "InvalidID");
    *whyFailed << "Principal::getByProductID: no product with branch type: "
               << branchType_ << " product id: " << pid << '\n';
    return GroupQueryResult{whyFailed};
  }

  cet::exempt_ptr<Group>
  Principal::getGroupLocal(ProductID const pid) const
  {
    std::lock_guard sentry{groupMutex_};
    auto it = groups_.find(pid);
    return it != groups_.cend() ? it->second.get() : nullptr;
  }

  cet::exempt_ptr<Group>
  Principal::getGroupTryAllFiles(ProductID const pid) const
  {
    // Look through current process and currently opened primary input file.
    if (producedInProcess(pid) || presentFromSource(pid)) {
      return getGroupLocal(pid);
    }
    // Look through secondary files
    for (auto const& sp : secondaryPrincipals_) {
      if (sp->presentFromSource(pid)) {
        return sp->getGroupLocal(pid);
      }
    }
    // Try new secondary files
    while (auto sp = tryNextSecondaryFile()) {
      auto& new_sp = secondaryPrincipals_.emplace_back(move(sp));
      if (new_sp->presentFromSource(pid)) {
        return new_sp->getGroupLocal(pid);
      }
    }
    return nullptr;
  }

} // namespace art
