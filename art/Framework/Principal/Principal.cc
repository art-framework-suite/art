#include "art/Framework/Principal/Principal.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/OutputHandle.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/Parentage.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "canvas/Utilities/Exception.h"
#include "canvas_root_io/Utilities/getWrapperTIDs.h"
#include "cetlib/container_algorithms.h"

#include <algorithm>
#include <cassert>
#include <mutex>
#include <utility>

using namespace cet;
using namespace std;

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
}

namespace art {

  class EventPrincipal;

  namespace {

    unique_ptr<Group>
    create_group(Principal* principal, DelayedReader* reader, BranchDescription const& bd)
    {
      unique_ptr<Group> result;
      auto tids = root::getWrapperTIDs(bd.producedClassName());
      switch (tids.size()) {
      case 1ull:
        // Standard Group.
        result = unique_ptr<Group>(new Group(principal, reader, bd, make_unique<RangeSet>(), tids[0]));
        break;
      case 2ull:
        // Assns<A, B, void>.
        result = unique_ptr<Group>(new Group(principal, reader, bd, make_unique<RangeSet>(), tids[0], tids[1]));
        break;
      case 4ull:
        // Assns<A, B, D>.
        result = unique_ptr<Group>(new Group(principal, reader, bd, make_unique<RangeSet>(), tids[0], tids[1], tids[2], tids[3]));
        break;
      default:
        // throw internal error exception
        throw Exception(errors::LogicError, "INTERNAL ART ERROR")
          << "While making groups, internal function getWrapperTIDs() returned an unexpected answer of size "
          << tids.size()
          << ".\n";
      }
      return result;
    }

  } // unnamed namespace

  Principal::~Principal() noexcept
  {
  }

  void
  Principal::ctor_create_groups(cet::exempt_ptr<ProductTable const> presentProducts)
  {
    if (!presentProducts) return;

    // Note: Dropped products are a problem. We should not create
    //       groups for them now because later we may open a secondary
    //       file which actually contains them and we want the
    //       secondary principal to have those groups. However some
    //       code expects to be able to find a group for dropped
    //       products, so getGroupTryAllFiles ignores groups for
    //       dropped products instead.
    for (auto const& pr : presentProducts->descriptions) {
      auto const& pd = pr.second;
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
      }
      else {
        // We have an old format file, convert.
        g->setProductProvenance(make_unique<ProductProvenance>(iter->productID(), productstatus::dummyToPreventDoubleCount(),
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
                       std::unique_ptr<DelayedReader>&& reader)
    : branchType_{branchType}
    , processConfiguration_{pc}
    , presentProducts_{presentProducts}
    , delayedReader_{std::move(reader)}
  {
    delayedReader_->setPrincipal(this);
    ctor_create_groups(presentProducts);
    ctor_read_provenance();
    ctor_fetch_process_history(hist);
  }

  // Run
  Principal::Principal(RunAuxiliary const& aux,
                       ProcessConfiguration const& pc,
                       cet::exempt_ptr<ProductTable const> presentProducts,
                       std::unique_ptr<DelayedReader>&& reader /* = std::make_unique<NoDelayedReader>() */)
    : branchType_{InRun}
    , processConfiguration_{pc}
    , presentProducts_{presentProducts}
    , delayedReader_{std::move(reader)}
    , runAux_{aux}
  {
    delayedReader_->setPrincipal(this);
    ctor_create_groups(presentProducts);
    ctor_read_provenance();
    ctor_fetch_process_history(runAux_.processHistoryID());
  }

  // SubRun
  Principal::Principal(SubRunAuxiliary const& aux,
                       ProcessConfiguration const& pc,
                       cet::exempt_ptr<ProductTable const> presentProducts,
                       std::unique_ptr<DelayedReader>&& reader /* = std::make_unique<NoDelayedReader>() */)
    : branchType_{InSubRun}
    , processConfiguration_{pc}
    , presentProducts_{presentProducts}
    , delayedReader_{std::move(reader)}
    , subRunAux_{aux}
  {
    delayedReader_->setPrincipal(this);
    ctor_create_groups(presentProducts);
    ctor_read_provenance();
    ctor_fetch_process_history(subRunAux_.processHistoryID());
  }

  // Event
  Principal::Principal(EventAuxiliary const& aux,
                       ProcessConfiguration const& pc,
                       cet::exempt_ptr<ProductTable const> presentProducts,
                       std::unique_ptr<History>&& history /* = std::make_unique<History>() */,
                       std::unique_ptr<DelayedReader>&& reader /* = std::make_unique<NoDelayedReader>() */,
                       bool const lastInSubRun /* = false */)
    : branchType_{InEvent}
    , processConfiguration_{pc}
    , presentProducts_{presentProducts}
    , delayedReader_{std::move(reader)}
    , eventAux_{aux}
    , history_{move(history)}
    , lastInSubRun_{lastInSubRun}
  {
    delayedReader_->setPrincipal(this);
    ctor_create_groups(presentProducts);
    ctor_read_provenance();
    ctor_fetch_process_history(history_->processHistoryID());
  }

  // Results
  Principal::Principal(ResultsAuxiliary const& aux,
                       ProcessConfiguration const& pc,
                       cet::exempt_ptr<ProductTable const> presentProducts,
                       std::unique_ptr<DelayedReader>&& reader /* = std::make_unique<NoDelayedReader>() */)
    : branchType_{InResults}
    , processConfiguration_{pc}
    , presentProducts_{presentProducts}
    , delayedReader_{std::move(reader)}
    , resultsAux_{aux}
  {
    delayedReader_->setPrincipal(this);
    ctor_create_groups(presentProducts);
    ctor_read_provenance();
    ctor_fetch_process_history(resultsAux_.processHistoryID());
  }

  void
  Principal::fillGroup(BranchDescription const& pd)
  {
    auto it = groups_.find(pd.productID());
    if (it != std::cend(groups_)) {
      // The 'combinable' call does not require that the processing
      // history be the same, which is not what we are checking for here.
      auto const& found_pd = it->second->productDescription();
      if (combinable(found_pd, pd)) {
        throw Exception(errors::Configuration)
          << "The process name "
          << pd.processName()
          << " was previously used on these products.\n"
          << "Please modify the configuration file to use a "
          << "distinct process name.\n";
      }

      throw Exception(errors::ProductRegistrationFailure)
        << "The product ID " << pd.productID()
        << " of the new product:\n"
        << pd
        << " collides with the product ID of the already-existing product:\n"
        << found_pd
        << "Please modify the instance name of the new product so as to avoid the product ID collision.\n"
        << "In addition, please notify artists@fnal.gov of this error.\n";
    }

    unique_ptr<Group> group = create_group(this, delayedReader_.get(), pd);
    groups_[pd.productID()] = move(group);
  }

  // Used by addToProcessHistory()
  void
  Principal::setProcessHistoryIDcombined(ProcessHistoryID const& phid)
  {
    if (branchType_ == InRun) {
      runAux_.setProcessHistoryID(phid);
    }
    else if (branchType_ == InSubRun) {
      subRunAux_.setProcessHistoryID(phid);
    }
    else if (branchType_ == InEvent) {
      history_->setProcessHistoryID(phid);
    }
    else {
      resultsAux_.setProcessHistoryID(phid);
    }
  }

  // Note: Used by run, subrun, event, and results ProductGetter!
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

  // Note: Used only by canvas RefCoreStreamer.cc through
  //       PrincipalBase::getEDProductGetter!
  EDProductGetter const*
  Principal::getEDProductGetter_(ProductID const& pid) const
  {
    return productGetter(pid);
  }

  //Note: To make secondary file-reading thread-safe, we will need
  //      to ensure that any adjustments to the secondaryPrincipals_
  //      vector is done atomically with respect to any reading of
  //      the vector.
  void
  Principal::addSecondaryPrincipal(std::unique_ptr<Principal>&& val)
  {
    secondaryPrincipals_.emplace_back(std::move(val));
  }

  void
  Principal::setProducedProducts(ProductTables const& producedProducts)
  {
    auto const& produced = producedProducts.get(branchType_);
    if (produced.descriptions.empty()) return;

    // The process history is expanded if there is a product that is
    // produced in this process.
    addToProcessHistory();

    producedProducts_ = cet::make_exempt_ptr(&produced);
    for (auto const& pr : produced.descriptions) {
      auto const& pd = pr.second;
      assert(pd.branchType() == branchType_);
      fillGroup(pd);
    }
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
    // FIXME: threading: For right now ignore the delay reading option
    // FIXME: threading: for product provenance. If we do the delay
    // FIXME: threading: reading then we must use a lock/mutex to
    // FIXME: threading: interlock all fetches of provenance because
    // FIXME: threading: the delay read fills the pp_by_pid_ one entry
    // FIXME: threading: at a time, and we do not want other threads
    // FIXME: threading: to find the info only partly there.
    for (auto const& pid_and_group : groups_) {
      auto group = pid_and_group.second.get();
      group->resolveProductIfAvailable();
    }
  }

  ProcessHistory const&
  Principal::processHistory() const
  {
    // Note: threading: We make no attempt to protect callers who use this
    // Note: threading: call to get access to the iteration interface of
    // Note: threading: the process history.  See the threading notes there
    // Note: threading: and here for the reasons why.
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
    return groups_.size();
  }

  Principal::const_iterator
  Principal::begin() const
  {
    return groups_.begin();
  }

  Principal::const_iterator
  Principal::cbegin() const
  {
    return groups_.cbegin();
  }

  Principal::const_iterator
  Principal::end() const
  {
    return groups_.end();
  }

  Principal::const_iterator
  Principal::cend() const
  {
    return groups_.cend();
  }

  // This is intended to be used by a module that fetches a very
  // large data product, makes a copy, and would like to release
  // the memory held by the original immediately.
  void
  Principal::removeCachedProduct(ProductID const pid) const
  {
    //FIXME: May be called by a module task, need to protect
    //FIXME: the group with a lock.
    if (auto g = getGroupLocal(pid)) {
      g->removeCachedProduct();
      return;
    }
    for (auto const& sp : secondaryPrincipals_) {
      if (auto g = sp->getGroupLocal(pid)) {
        g->removeCachedProduct();
        return;
      }
    }
    throw Exception(errors::ProductNotFound, "removeCachedProduct")
      << "Attempt to remove unknown product corresponding to ProductID: " << pid << '\n'
      << "Please contact artists@fnal.gov\n";
  }

  // Used by Principal::put to insert the data product provenance.
  // Used by RootDelayedReader::getProduct_ to replace the product provenance when merging run and subRun data products.
  void
  Principal::insert_pp(std::unique_ptr<ProductProvenance const>&& pp)
  {
    auto const& pid = pp->productID();
    auto g = getGroupLocal(pid);
    if (g.get() != nullptr) {
      g->setProductProvenance(move(pp));
    }
  }

  cet::exempt_ptr<ProductProvenance const>
  Principal::branchToProductProvenance(ProductID const& pid) const
  {
    // FIXME: threading: For right now ignore the delay reading option
    // FIXME: threading: for product provenance. If we do the delay
    // FIXME: threading: reading then we must use a lock/mutex to
    // FIXME: threading: interlock all fetches of provenance because
    // FIXME: threading: the delay read fills the pp_by_pid_ one entry
    // FIXME: threading: at a time, and we do not want other threads
    // FIXME: threading: to find the info only partly there.
    // Note: This routine may lock the input source mutex.
    cet::exempt_ptr<ProductProvenance const> ret;
    auto g = getGroupLocal(pid);
    if (g.get() != nullptr) {
      ret = g->productProvenance();
    }
    return ret;
  }

  //
  // Note: threading: The solution chosen for the following problems is to convert groups_ from type:
  //
  //            std::map<ProductID, std::unique_ptr<Group>>
  //
  //       to type:
  //
  //            tbb::concurrent_unordered_map<ProductID, std::unique_ptr<Group>>
  //
  //       We get concurrent insertion and iteration, but not concurrent erasure (which is not a problem
  //       because we never remove groups).  Note that tbb uses a value for the end() iterator for this
  //       class which is always valid for comparing against the result of an interation or find (it is
  //       implemented as (<element-ptr>(nullptr), <internal-table>)).
  //
  // Note: threading: May be called from producer and filter module processing tasks! This requires us to
  //       protect groups_ against multiple threads attempting an insert at the same time.
  //
  // Note: threading: Also anyone using the iterators over groups_ (which are Principal::begin() and Principal::end())
  //       need protection against having their interators invalidated.  Right now the only code doing this is:
  //
  //            Principal::readImmediate() const
  //            Principal::getGroupTryAllFiles(ProductID const& pid) const
  //            Principal::removeCachedProduct(ProductID const pid) const
  //            Principal::findGroupsForProcess(...) const
  //            OutputModule::updateBranchChildren()
  //
  //       Principal::readImmediate() is called just after principal creation with the input mutex held.  Module tasks
  //       on other streams could be doing puts which would invalid the iterator if the data product being put does
  //       not exist in the input file and is being put for the first time so this is a group insertion.
  //
  //       Principal::getGroupTryAllFiles(ProductID const& pid) const
  //         Used by Principal::getByProductID(ProductID const& pid) const
  //           Used by art::DataViewImpl<T>::get(ProductID const pid, Handle<T>& result) const. (easy user-facing api)
  //           Used by Principal::productGetter(ProductID const pid) const
  //             Used by (Run,SubRun,Event,Results)::productGetter (advanced user-facing api)
  //         Used by Principal::getForOutput(ProductID const pid, bool resolveProd) const
  //           Used by RootOutputFile to fetch products being written to disk.
  //           Used by FileDumperOutput_module.
  //           Used by ProvenanceCheckerOutput_module.
  //       These uses are find() and compare against end().  Problem is that end() may have moved by the time
  //       we do the compare with the find result. There is also use of the mpr and secondary files.
  //
  //       Principal::removeCachedProduct(ProductID const pid) const
  //       Principal::findGroupsForProcess(...) const
  //       These uses are find() and compare against end().  Problem is that end() may have moved by the time
  //       we do the compare with the find result. There is also use of secondary principals.
  //
  //       OutputModule::updateBranchChildren is called only for events and only after all module
  //       processing tasks which can put products into the event have finished running, so it does not need
  //       the protection.
  //

  // Used by the Run, SubRun, and EventPrincipal constructors if a product was produced.
  // Used by RootOutput_module from write, writeSubRun, and writeRun if a branch was dropped by selectEvents processing
  //                                                                 for that kind of principal.
  // Used by RootOutput_module from startEndFile if a branch was dropped by selectEvents processing for a results principal,
  //                                             or if a product was produced by a results principal (note that it has not
  //                                             actually gotten the chance to make the product, that happens right after
  //                                             this call is made).
  //
  // Note: threading: If the only uses were from the constructors we would have no problems, but the use from
  //       the root output module is bad because it could be running concurrently with other output modules
  //       and analyzers for this same principal. So we have to use a compare_exchange_strong on processHistoryModified_
  //       so that only one task tries to do this. We also need to stall the other output and analyzer modules that
  //       call processHistory() while we are doing the update so that they get the updated result.  For output and
  //       analyzer modules that have already fetched the process history pointer, we have to stall any attempt to
  //       access its internal process configuration list while we are updating it.
  //
  // FIXME: threading: We hand out processHistory_ through the processHistory() interface, which is in turn
  // FIXME: threading: handed out by the DataViewImpl::processHistory() interface to any module task that wants it.
  // FIXME: threading: This is a problem for output modules and analyzers if an output module decides to update the
  // FIXME: threading: process history from startEndFile. We must stall users of the process history if we updating it,
  // FIXME: threading: both by stalling a fetch of processHistory_ once we start to update, and for those modules
  // FIXME: threading: that have already fetched the processHistory_ we must stall any attempt by them to access
  // FIXME: threading: its internal process configuration list while we are changing it.
  void
  Principal::addToProcessHistory()
  {
    bool expected = false;
    if (processHistoryModified_.compare_exchange_strong(expected, true)) {
      // Note: threading: We have now locked out any other task trying to modify the process history.
      // Note: threading: Now we have to block tasks that already have a pointer to the process history
      // Note: threading: from accessing its internals while we update it.
      // Note: threading: Note: We do not protect the iteration interface, the begin(), end(), and size()
      // Note: threading: Note: are all separate calls and we cannot lock the mutex in each one because
      // Note: threading: Note: there is no way to automatically unlock it.
      lock_guard<recursive_mutex> sentry(processHistory_.get_mutex());
      string const& processName = processConfiguration_.processName();
      for (auto const& val : processHistory_) {
        if (processName == val.processName()) {
          throw art::Exception(errors::Configuration)
            << "The process name "
            << processName
            << " was previously used on these products.\n"
            << "Please modify the configuration file to use a "
            << "distinct process name.\n";
        }
      }
      processHistory_.push_back(processConfiguration_);
      // OPTIMIZATION NOTE: As of 0_9_0_pre3 For very simple Sources
      // (e.g. EmptyEvent) this routine takes up nearly 50% of the time
      // per event, and 96% of the time for this routine is spent in
      // computing the ProcessHistory id which happens because we are
      // reconstructing the ProcessHistory for each event.  It would
      // probably be better to move the ProcessHistory construction out to
      // somewhere which persists for longer than one Event.
      auto const phid = processHistory_.id();
      ProcessHistoryRegistry::emplace(phid, processHistory_);
      // Note: threading: We must protect processHistory_!  The id() call can modify it!
      // Note: threading: We are modifying Run, SubRun, Event, and Results principals here, and their *Auxiliary
      // Note: threading: and the event principal art::History.
      setProcessHistoryIDcombined(processHistory_.id());
    }
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
  Principal::getByLabel(WrappedTypeID const& wrapped,
                        string const& label,
                        string const& productInstanceName,
                        string const& processName) const
  {
    Selector const sel{ModuleLabelSelector{label} &&
                       ProductInstanceNameSelector{productInstanceName} &&
                       ProcessNameSelector{processName}};
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

  Principal::GroupQueryResultVec
  Principal::getMatchingSequence(SelectorBase const& selector) const
  {
    GroupQueryResultVec results;

    // Find groups from current process
    if (producedProducts_) {
      if (findGroups(producedProducts_->viewLookup, selector, results, true) != 0) {
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
    GroupQueryResultVec results;
    if (!presentProducts_) {
      return results;
    }

    findGroups(presentProducts_->viewLookup, selector, results, true);
    return results;
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

    // Loop over processes in reverse time order.  Sometimes we want to stop
    // after we find a process with matches so check for that at each step.
    // Note: threading: We must protect the process history iterators here
    // Note: threading: against possible invalidation by output modules inserting
    // Note: threading: a process history entry while we are iterating.
    lock_guard<recursive_mutex> sentry{processHistory_.get_mutex()};
    for (auto const& h : reverse_iteration(processHistory_)) {
      auto it = pl.find(h.processName());
      if (it != pl.end()) {
        found += findGroupsForProcess(it->second, sel, res, wanted_wrapper);
      }
      if (stopIfProcessHasMatch && !res.empty())  break;
    }
    return found;
  }

  std::size_t
  Principal::findGroupsFromInputFile(WrappedTypeID const& wrapped,
                                     SelectorBase const& selector,
                                     GroupQueryResultVec& results,
                                     bool const stopIfProcessHasMatch) const
  {
    if (!presentProducts_) {
      return 0;
    }
    auto const& lookup = presentProducts_->productLookup;
    auto it = lookup.find(wrapped.product_type.friendlyClassName());
    if (it == lookup.end()) {
      return 0;
    }
    return findGroups(it->second, selector, results, stopIfProcessHasMatch, wrapped.wrapped_product_type);
  }

  Principal::GroupQueryResultVec
  Principal::findGroupsForProduct(WrappedTypeID const& wrapped,
                                  SelectorBase const& selector,
                                  bool const stopIfProcessHasMatch) const
  {
    GroupQueryResultVec results;

    unsigned ret{};
    // Find groups from current process
    if (producedProducts_) {
      auto const& lookup = producedProducts_->productLookup;
      auto it = lookup.find(wrapped.product_type.friendlyClassName());
      if (it != lookup.end()) {
        ret += findGroups(it->second, selector, results, stopIfProcessHasMatch, wrapped.wrapped_product_type);
      }
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
  Principal::findGroupsForProcess(std::vector<ProductID> const& vpid,
                                  SelectorBase const& sel,
                                  GroupQueryResultVec& res,
                                  TypeID const wanted_wrapper) const
  {
    std::size_t found{}; // Horrible hack that should go away
    for (auto const pid : vpid) {
      auto group = getGroupLocal(pid);
      if (!group) {
        continue;
      }
      if (!sel.match(group->productDescription())) {
        continue;
      }
      if (!group->tryToResolveProduct(wanted_wrapper)) {
        continue;
      }
      // Found a good match, save it.
      res.emplace_back(group);
      ++found;
    }
    return found;
  }

  SubRunPrincipal const&
  Principal::subRunPrincipal() const
  {
    if (!subRunPrincipal_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL subRunPrincipal.\n";
    }
    return *subRunPrincipal_;
  }

  cet::exempt_ptr<RunPrincipal const>
  Principal::runPrincipalExemptPtr() const
  {
    return runPrincipal_;
  }

  cet::exempt_ptr<SubRunPrincipal const>
  Principal::subRunPrincipalExemptPtr() const
  {
    return subRunPrincipal_;
  }

  void
  Principal::setRunPrincipal(cet::exempt_ptr<RunPrincipal const> rp)
  {
    runPrincipal_ = rp;
  }

  void
  Principal::setSubRunPrincipal(cet::exempt_ptr<SubRunPrincipal const> srp)
  {
    subRunPrincipal_ = srp;
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

  bool
  Principal::isReal() const
  {
    return eventAux_.isRealData();
  }

  EventAuxiliary::ExperimentType
  Principal::ExperimentType() const
  {
    return eventAux_.experimentType();
  }

  History const&
  Principal::history() const
  {
    return *history_;
  }

  EventSelectionIDVector const&
  Principal::eventSelectionIDs() const
  {
    return history_->eventSelectionIDs();
  }

  RunPrincipal const&
  Principal::runPrincipal() const
  {
    if (!runPrincipal_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL runPrincipal.\n";
    }
    return *runPrincipal_;
  }


  bool
  Principal::isLastInSubRun() const
  {
    return lastInSubRun_;
  }

  void
  Principal::put(BranchDescription const& bd,
                 unique_ptr<ProductProvenance const>&& pp,
                 unique_ptr<EDProduct>&& edp,
                 unique_ptr<RangeSet>&& rs)
  {
    assert(edp);
    if ((branchType_ == InRun) || (branchType_ == InSubRun)) {
      // Note: We intentionally allow group and provenance replacement
      //       for run and subrun products.
      auto group = getGroupLocal(bd.productID());
      assert(group);
      group->setProductAndProvenance(move(pp), move(edp), move(rs));
    }
    else {
      auto group = getGroupLocal(bd.productID());
      assert(group);
      if (group->anyProduct() != nullptr) {
        throw art::Exception(art::errors::ProductRegistrationFailure, "Principal::put:")
          << "Problem found during put of "
          << branchType_
          << " product: product already put for "
          << bd.branchName()
          << '\n';
      }
      group->setProductAndProvenance(move(pp), move(edp), make_unique<RangeSet>());
    }
  }

  // Used by RootOutputFile to fetch products being written to disk.
  // Used by FileDumperOutput_module.
  // Used by ProvenanceCheckerOutput_module.
  // We invoke the delay reader now if no user module has ever fetched them
  // for this principal if resolvedProd is true.
  // Note: This attempts to resolve the product and converts
  // Note: the resulting group into an OutputHandle.
  // Note: threading: Right now this is single-threaded.  Be careful if this changes!!!
  OutputHandle
  Principal::getForOutput(ProductID const& pid, bool resolveProd) const
  {
    // FIXME: threading: Uses of group!
    auto g = getGroupTryAllFiles(pid);
    if (g.get() == nullptr) {
      return OutputHandle{RangeSet::invalid()};
    }

    if (resolveProd) {
      bool gotIt = g->resolveProductIfAvailable();
      if (!gotIt) {
        // Behavior is the same as if the group wasn't there.
        return OutputHandle{RangeSet::invalid()};
      }
    }
    auto const& pd = g->productDescription();
    if (resolveProd && // asked to resolve
        !g->anyProduct()->isPresent() && // wrapper says it is a dummy, and
        (presentFromSource(pid) || pd.produced()) && // , and
        productstatus::present(g->productProvenance()->productStatus()) // provenance says present
        ){
      throw Exception(errors::LogicError, "Principal::getForOutput\n")
        << "A product with a status of 'present' is not actually present.\n"
        << "The branch name is "
        << g->productDescription().branchName()
        << "\nContact a framework developer.\n";
    }
    if (!g->anyProduct() && !g->productProvenance()) {
      return OutputHandle{g->rangeOfValidity()};
    }
    return OutputHandle{g->anyProduct(), &g->productDescription(), g->productProvenance(), g->rangeOfValidity()};
  }

  cet::exempt_ptr<BranchDescription const>
  Principal::getProductDescription(ProductID const pid) const
  {
    // Find groups from current process
    if (producedProducts_) {
      if (auto result = producedProducts_->description(pid)) {
        return result;
      }
    }

    if (presentProducts_) {
      // Look through currently opened input files
      if (auto result = presentProducts_->description(pid)) {
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

  // Used by RootOutputFile
  // Used by Run
  RunAuxiliary const&
  Principal::runAux() const
  {
    return runAux_;
  }

  SubRunAuxiliary const&
  Principal::subRunAux() const
  {
    return subRunAux_;
  }

  EventAuxiliary const&
  Principal::eventAux() const
  {
    return eventAux_;
  }

  ResultsAuxiliary const&
  Principal::resultsAux() const
  {
    return resultsAux_;
  }

  RunID const&
  Principal::runID() const
  {
    return runAux_.id();
  }

  SubRunID
  Principal::subRunID() const
  {
    return subRunAux_.id();
  }

  EventID const&
  Principal::eventID() const
  {
    return eventAux_.id();
  }

  RunNumber_t
  Principal::run() const
  {
    if (branchType_ == InRun) {
      return runAux_.run();
    }
    if (branchType_ == InSubRun) {
      return subRunAux_.run();
    }
    return eventAux_.id().run();
  }

  SubRunNumber_t
  Principal::subRun() const
  {
    if (branchType_ == InSubRun) {
      return subRunAux_.subRun();
    }
    return eventAux_.id().subRun();
  }

  EventNumber_t
  Principal::event() const
  {
    return eventAux_.id().event();
  }

  Timestamp const&
  Principal::beginTime() const
  {
    if (branchType_ == InRun) {
      return runAux_.beginTime();
    }
    return subRunAux_.beginTime();
  }

  // Used by EventProcessor
  Timestamp const&
  Principal::endTime() const
  {
    if (branchType_ == InRun) {
      return runAux_.endTime();
    }
    return subRunAux_.endTime();
  }

  void
  Principal::endTime(Timestamp const& time)
  {
    if (branchType_ == InRun) {
      runAux_.endTime(time);
      return;
    }
    subRunAux_.setEndTime(time);
  }

  Timestamp const&
  Principal::time() const
  {
    return eventAux_.time();
  }



  int
  Principal::tryNextSecondaryFile() const
  {
    int const err = delayedReader_->openNextSecondaryFile(nextSecondaryFileIdx_);
    if (err != -2) {
      // there are more files to try
      ++nextSecondaryFileIdx_;
    }
    return err;
  }

  bool
  Principal::producedInProcess(ProductID const pid) const
  {
    if (!producedProducts_) {
      return false;
    }

    auto pd = producedProducts_->description(pid);
    return pd == nullptr ? false : pd->produced();
  }

  bool
  Principal::presentFromSource(ProductID const pid) const
  {
    if (!presentProducts_) {
      return false;
    }

    auto pd = presentProducts_->description(pid);
    return pd == nullptr ? false : pd->present();
  }

  // Used by art::DataViewImpl<T>::get(ProductID const pid, Handle<T>& result) const. (easy user-facing api)
  // Used by Principal::productGetter(ProductID const pid) const
  //   Used by (Run,SubRun,Event,Results)::productGetter (advanced user-facing api)
  GroupQueryResult
  Principal::getByProductID(ProductID const pid) const
  {
    if (auto const g = getGroupTryAllFiles(pid)) {
      return GroupQueryResult{g};
    }
    auto whyFailed = make_shared<Exception>(errors::ProductNotFound, "InvalidID");
    *whyFailed << "Principal::getByProductID: no product with branch type: " << branchType_ << " product id: " << pid << "\n";
    return GroupQueryResult{whyFailed};
  }

  cet::exempt_ptr<Group>
  Principal::getGroupLocal(ProductID const pid) const
  {
    auto it = groups_.find(pid);
    return it != groups_.cend() ? it->second.get() : nullptr;
  }

  // Used by Principal::getByProductID(ProductID const& pid) const
  //   Used by art::DataViewImpl<T>::get(ProductID const pid, Handle<T>& result) const. (easy user-facing api)
  //   Used by Principal::productGetter(ProductID const pid) const
  //     Used by (Run,SubRun,Event,Results)::productGetter (advanced user-facing api)
  // Used by Principal::getForOutput(ProductID const& pid, bool resolveProd) const
  //   Used by RootOutputFile to fetch products being written to disk.
  //   Used by FileDumperOutput_module.
  //   Used by ProvenanceCheckerOutput_module.
  cet::exempt_ptr<Group const>
  Principal::getGroupTryAllFiles(ProductID const pid) const
  {
    // bool produced{false};
    // if (producedProducts_) {
    //   auto const& availableProducts = producedProducts_->availableProducts;
    //   if (availableProducts.find(pid) != availableProducts.cend()) {
    //     produced = true;
    //   }
    // }

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
        return new_sp->getGroupLocal(pid);
      }
    }

    return nullptr;
  }

} // namespace art
