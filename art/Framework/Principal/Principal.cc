#include "art/Framework/Principal/Principal.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/OutputHandle.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Persistency/Provenance/detail/type_aliases.h"
#include "canvas/Persistency/Common/PrincipalBase.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventRange.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/Parentage.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/ResultsAuxiliary.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/TypeTools.h"
#include "canvas/Persistency/Provenance/TypeWithDict.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/value_ptr.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "tbb/concurrent_unordered_map.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace cet;
using namespace std;

namespace art {

  class EventPrincipal;

  namespace {

    void
    maybeThrowLateDictionaryError(art::TypeWithDict const& twd, std::string const& tname)
    {
      if (!twd) {
        art::throwLateDictionaryError(tname);
      }
    }

    vector<TypeID>
    getWrapperTIDs(string const& productClassName)
    {
      vector<TypeID> result;
      // Type of product.
      TypeWithDict const ta(productClassName);
      maybeThrowLateDictionaryError(ta, productClassName);
      // Wrapper of product.
      auto const twName = wrappedClassName(productClassName);
      TypeWithDict const tw(twName);
      maybeThrowLateDictionaryError(tw, twName);
      result.emplace_back(tw.id());
      if (ta.category() == TypeWithDict::Category::CLASSTYPE && is_instantiation_of(ta.tClass(), "art::Assns")) {
        // Type of assns partner.
        auto const tpName = name_of_assns_partner(productClassName);
        TypeWithDict const tp(tpName);
        maybeThrowLateDictionaryError(tp, tpName);
        // Wrapper of assns partner.
        auto const twpName = wrappedClassName(tpName);
        TypeWithDict const twp(twpName);
        maybeThrowLateDictionaryError(twp, twpName);
        result.emplace_back(twp.id());
        auto const baseName = name_of_assns_base(productClassName);
        if (!baseName.empty()) {
          // Type of assns base.
          TypeWithDict const base(baseName);
          maybeThrowLateDictionaryError(base, baseName);
          // Wrapper of assns base.
          auto const basewName = wrappedClassName(baseName);
          TypeWithDict const basew(basewName);
          maybeThrowLateDictionaryError(basew, basewName);
          result.emplace_back(basew.id());
          // Type of assns base partner.
          auto const basepName = name_of_assns_partner(baseName);
          TypeWithDict const basep(basepName);
          maybeThrowLateDictionaryError(basep, basepName);
          // Wrapper of assns base partner.
          auto const basewpName = wrappedClassName(basepName);
          TypeWithDict const basewp(basewpName);
          maybeThrowLateDictionaryError(basewp, basewpName);
          result.emplace_back(basewp.id());
        }
      }
      return result;
    }

    unique_ptr<Group>
    create_group(Principal* principal, DelayedReader* reader, BranchDescription const& bd)
    {
      unique_ptr<Group> result;
      auto tids = getWrapperTIDs(bd.producedClassName());
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

  Principal::
  ~Principal() noexcept
  {
  }

  void
  Principal::
  ctor_create_groups()
  {
    // Note: Dropped products are a problem. We should not create groups for them now because later
    //       we may open a secondary file which actually contains them and we want the secondary
    //       principal to have those groups. However some code expects to be able to find a group
    //       for dropped products, so getGroupTryAllFiles ignores groups for dropped products instead.
    {
      auto const& pmd = ProductMetaData::instance();
      for (auto const& val : pmd.productList()) {
        auto const& bd = val.second;
        if (bd.branchType() != branchType_) {
          continue;
        }
        unique_ptr<Group> group = create_group(this, delayedReader_.get(), bd);
        groups_[bd.productID()] = move(group);
      }
    }
  }

  void
  Principal::
  ctor_read_provenance()
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
  Principal::
  ctor_fetch_process_history(ProcessHistoryID const& phid)
  {
    if (!phid.isValid()) {
      return;
    }
    ProcessHistory processHistory;
    ProcessHistoryRegistry::get(phid, processHistory);
    std::swap(*processHistoryPtr_, processHistory);
  }

  void
  Principal::
  ctor_add_to_process_history()
  {
    if (ProductMetaData::instance().productProduced(branchType_)) {
      addToProcessHistory();
    }
  }

  Principal::
  Principal(BranchType branchType,
            ProcessConfiguration const& pc,
            ProcessHistoryID const& hist,
            std::unique_ptr<DelayedReader>&& reader)
    : PrincipalBase()
    , branchType_{branchType}
    , processHistoryPtr_{std::make_shared<ProcessHistory>()}
    , processHistoryModified_{false}
    , processConfiguration_{pc}
    , groups_{}
    , delayedReader_{std::move(reader)}
    , secondaryPrincipals_{}
    , nextSecondaryFileIdx_{}
  {
    delayedReader_->setPrincipal(this);
    ctor_create_groups();
    ctor_read_provenance();
    ctor_fetch_process_history(hist);
  }

  // Run
  Principal::
  Principal(RunAuxiliary const& aux,
            ProcessConfiguration const& pc,
            std::unique_ptr<DelayedReader>&& reader /* = std::make_unique<NoDelayedReader>() */)
    : PrincipalBase()
    , branchType_{InRun}
    , processHistoryPtr_{std::make_shared<ProcessHistory>()}
    , processHistoryModified_{false}
    , processConfiguration_{pc}
    , groups_{}
    , delayedReader_{std::move(reader)}
    , secondaryPrincipals_{}
    , nextSecondaryFileIdx_{}
    , rangeSet_{RangeSet::invalid()}
    , runAux_{aux}
    , subRunAux_{}
    , eventAux_{}
    , resultsAux_{}
    , runPrincipal_{nullptr}
    , subRunPrincipal_{nullptr}
    , history_{}
    , lastInSubRun_{false}
  {
    delayedReader_->setPrincipal(this);
    ctor_create_groups();
    ctor_read_provenance();
    ctor_fetch_process_history(runAux_.processHistoryID());
    ctor_add_to_process_history();
  }

  // SubRun
  Principal::
  Principal(SubRunAuxiliary const& aux, ProcessConfiguration const& pc,
            std::unique_ptr<DelayedReader>&& reader /* = std::make_unique<NoDelayedReader>() */)
    : PrincipalBase()
    , branchType_{InSubRun}
    , processHistoryPtr_{std::make_shared<ProcessHistory>()}
    , processHistoryModified_{false}
    , processConfiguration_{pc}
    , groups_{}
    , delayedReader_{std::move(reader)}
    , secondaryPrincipals_{}
    , nextSecondaryFileIdx_{}
    , rangeSet_{RangeSet::invalid()}
    , runAux_{}
    , subRunAux_{aux}
    , eventAux_{}
    , resultsAux_{}
    , runPrincipal_{nullptr}
    , subRunPrincipal_{nullptr}
    , history_{}
    , lastInSubRun_{false}
  {
    delayedReader_->setPrincipal(this);
    ctor_create_groups();
    ctor_read_provenance();
    ctor_fetch_process_history(subRunAux_.processHistoryID());
    ctor_add_to_process_history();
  }

  // Event
  Principal::
  Principal(EventAuxiliary const& aux, ProcessConfiguration const& pc,
            std::unique_ptr<History>&& history /* = std::make_unique<History>() */,
            std::unique_ptr<DelayedReader>&& reader /* = std::make_unique<NoDelayedReader>() */,
            bool lastInSubRun /* = false */)
    : PrincipalBase()
    , branchType_{InEvent}
    , processHistoryPtr_{std::make_shared<ProcessHistory>()}
    , processHistoryModified_{false}
    , processConfiguration_{pc}
    , groups_{}
    , delayedReader_{std::move(reader)}
    , secondaryPrincipals_{}
    , nextSecondaryFileIdx_{}
    , rangeSet_{RangeSet::invalid()}
    , runAux_{}
    , subRunAux_{}
    , eventAux_{aux}
    , resultsAux_{}
    , runPrincipal_{nullptr}
    , subRunPrincipal_{nullptr}
    , history_{move(history)}
    , lastInSubRun_{lastInSubRun}
  {
    delayedReader_->setPrincipal(this);
    ctor_create_groups();
    ctor_read_provenance();
    ctor_fetch_process_history(history_->processHistoryID());
    ctor_add_to_process_history();
  }

  // Results
  Principal::
  Principal(ResultsAuxiliary const& aux, ProcessConfiguration const& pc,
            std::unique_ptr<DelayedReader>&& reader /* = std::make_unique<NoDelayedReader>() */)
    : PrincipalBase()
    , branchType_{InResults}
    , processHistoryPtr_{std::make_shared<ProcessHistory>()}
    , processHistoryModified_{false}
    , processConfiguration_{pc}
    , groups_{}
    , delayedReader_{std::move(reader)}
    , secondaryPrincipals_{}
    , nextSecondaryFileIdx_{}
    , rangeSet_{RangeSet::invalid()}
    , runAux_{}
    , subRunAux_{}
    , eventAux_{}
    , resultsAux_{aux}
    , runPrincipal_{nullptr}
    , subRunPrincipal_{nullptr}
    , history_{}
    , lastInSubRun_{false}
  {
    delayedReader_->setPrincipal(this);
    ctor_create_groups();
    ctor_read_provenance();
    ctor_fetch_process_history(resultsAux_.processHistoryID());
  }

  // Used by addToProcessHistory()
  void
  Principal::
  setProcessHistoryIDcombined(ProcessHistoryID const& phid)
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

  void
  Principal::
  fillGroup(BranchDescription const& bd)
  {
    unique_ptr<Group> group = create_group(this, delayedReader_.get(), bd);
    groups_[bd.productID()] = move(group);
  }

  // Note: Used by run, subrun, event, and results ProductGetter!
  // Note: LArSoft uses this extensively to create a Ptr by hand.
  EDProductGetter const*
  Principal::
  productGetter(ProductID const& pid) const
  {
    auto g = getGroupTryAllFiles(pid);
    if (g.get() != nullptr) {
      // Note: All produced products should be found.
      return g.get();
    }
    return nullptr;
  }

  // Note: Used only by canvas RefCoreStreamer.cc through PrincipalBase::getEDProductGetter!
  EDProductGetter const*
  Principal::
  getEDProductGetter_(ProductID const& pid) const
  {
    return productGetter(pid);
  }

  // This is intended to be used by a module that fetches a very
  // large data product, makes a copy, and would like to release
  // the memory held by the original immediately.
  void
  Principal::
  removeCachedProduct(ProductID const& pid) const
  {
    //FIXME: May be called by a module task, need to protect
    //FIXME: the group with a lock.
    cet::exempt_ptr<Group> g;
    {
      auto PI = groups_.find(pid);
      if (PI != groups_.end()) {
        g = PI->second.get();
      }
      else {
        for (auto& p : secondaryPrincipals_) {
          auto I = p->groups_.find(pid);
          if (I != p->groups_.end()) {
            g = I->second.get();
            break;
          }
        }
      }
    }
    if (g.get() == nullptr ) {
      throw Exception(errors::ProductNotFound, "removeCachedProduct")
        << "Attempt to remove unknown product corresponding to ProductID: "
        << pid
        << '\n'
        << "Please contact artists@fnal.gov\n";
    }
    g->removeCachedProduct();
  }

  void
  Principal::
  addSecondaryPrincipal(std::unique_ptr<Principal>&& val)
  {
    //Note: We are not trying to make secondary file reading
    //      thread-safe at the moment.
    secondaryPrincipals_.emplace_back(std::move(val));
  }

  void
  Principal::
  readImmediate() const
  {
    // Read all data products and provenance immediately, if available.
    // Used only by RootInputFile to implement the delayedRead*Products config options.
    //
    // Note: The input source lock will be held when this routine is called.
    //
    //readProvenanceImmediate();
    //{
    //  for (auto const& bidAndGroup : groups_) {
    //    (void) bidAndGroup.second->productProvenance();
    //  }
    //  branchMapperPtr_->setDelayedRead(false);
    //}
    // FIXME: threading: For right now ignore the delay reading option
    // FIXME: threading: for product provenance. If we do the delay
    // FIXME: threading: reading then we must use a lock/mutex to
    // FIXME: threading: interlock all fetches of provenance because
    // FIXME: threading: the delay read fills the pp_by_pid_ one entry
    // FIXME: threading: at a time, and we do not want other threads
    // FIXME: threading: to find the info only partly there.
    //if (pp_by_bid_needs_reading_) {
    //  pp_by_bid_needs_reading_ = false;
    //  auto ppv = branchMapperPtr_->readProvenance();
    //  for (auto iter = ppv.begin(), end = ppv.end(); iter != end; ++iter) {
    //    pp_by_pid_[iter->productID()].reset(new ProductProvenance(*iter));
    //  }
    //}
    for (auto const& pid_and_group : groups_) {
      auto group = pid_and_group.second.get();
      group->resolveProductIfAvailable();
    }
  }

  ProcessHistory const&
  Principal::
  processHistory() const
  {
    // Note: threading: We make no attempt to protect callers who use this
    // Note: threading: call to get access to the iteration interface of
    // Note: threading: the process history.  See the threading notes there
    // Note: threading: and here for the reasons why.
    return *processHistoryPtr_;
  }

  ProcessConfiguration const&
  Principal::
  processConfiguration() const
  {
    return processConfiguration_;
  }

  size_t
  Principal::
  size() const
  {
    return groups_.size();
  }

  Principal::const_iterator
  Principal::
  begin() const
  {
    return groups_.begin();
  }

  //Principal::const_iterator
  //Principal::
  //cbegin() const
  //{
  //  return groups_.cbegin();
  //}

  Principal::const_iterator
  Principal::
  end() const
  {
    return groups_.end();
  }

  //Principal::const_iterator
  //Principal::
  //cend() const
  //{
  //  return groups_.cend();
  //}

  // Used by Principal::put to insert the data product provenance.
  // Used by RootDelayedReader::getProduct_ to replace the product provenance when merging run and subRun data products.
  void
  Principal::
  insert_pp(std::unique_ptr<ProductProvenance const>&& pp)
  {
    auto const& pid = pp->productID();
    auto g = getGroupLocal(pid);
    if (g.get() != nullptr) {
      g->setProductProvenance(move(pp));
    }
  }

  cet::exempt_ptr<ProductProvenance const>
  Principal::
  branchToProductProvenance(ProductID const& pid) const
  {
    // FIXME: threading: For right now ignore the delay reading option
    // FIXME: threading: for product provenance. If we do the delay
    // FIXME: threading: reading then we must use a lock/mutex to
    // FIXME: threading: interlock all fetches of provenance because
    // FIXME: threading: the delay read fills the pp_by_pid_ one entry
    // FIXME: threading: at a time, and we do not want other threads
    // FIXME: threading: to find the info only partly there.
    // Note: This routine may lock the input source mutex.
    //if (pp_by_bid_needs_reading_) {
    //  pp_by_bid_needs_reading_ = false;
    //  auto ppv = branchMapperPtr_->readProvenance();
    //  for (auto iter = ppv.begin(), end = ppv.end(); iter != end; ++iter) {
    //    pp_by_pid_[iter->productID()].reset(new ProductProvenance(*iter));
    //  }
    //}
    cet::exempt_ptr<ProductProvenance const> ret;
    //auto iter = pp_by_pid_.find(pid);
    //if (iter != pp_by_pid_.end()) {
    //  ret = iter->second.get();
    //}
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
  // FIXME: threading: We hand out *processHistoryPtr_ through the processHistory() interface, which is in turn
  // FIXME: threading: handed out by the DataViewImpl::processHistory() interface to any module task that wants it.
  // FIXME: threading: This is a problem for output modules and analyzers if an output module decides to update the
  // FIXME: threading: process history from startEndFile. We must stall users of the process history if we updating it,
  // FIXME: threading: both by stalling a fetch of processHistoryPtr_ once we start to update, and for those modules
  // FIXME: threading: that have already fetched the processHistoryPtr_ we must stall any attempt by them to access
  // FIXME: threading: its internal process configuration list while we are changing it.
  void
  Principal::
  addToProcessHistory()
  {
    bool expected = false;
    if (processHistoryModified_.compare_exchange_strong(expected, true)) {
      // Note: threading: We have now locked out any other task trying to modify the process history.
      // Note: threading: Now we have to block tasks that already have a pointer to the process history
      // Note: threading: from accessing its internals while we update it.
      // Note: threading: Note: We do not protect the iteration interface, the begin(), end(), and size()
      // Note: threading: Note: are all separate calls and we cannot lock the mutex in each one because
      // Note: threading: Note: there is no way to automatically unlock it.
      lock_guard<recursive_mutex> sentry(processHistoryPtr_->get_mutex());
      string const& processName = processConfiguration_.processName();
      for (auto const& val : *processHistoryPtr_) {
        if (processName == val.processName()) {
          throw art::Exception(errors::Configuration)
            << "The process name "
            << processName
            << " was previously used on these products.\n"
            << "Please modify the configuration file to use a "
            << "distinct process name.\n";
        }
      }
      // Note: threading: We have protected processHistoryPtr_ by locking the mutex above.
      processHistoryPtr_->push_back(processConfiguration_);
      // OPTIMIZATION NOTE: As of 0_9_0_pre3
      // For very simple Sources (e.g. EmptyEvent) this routine takes up
      // nearly 50% of the time per event, and 96% of the time for this
      // routine is spent in computing the ProcessHistory id which happens
      // because we are reconstructing the ProcessHistory for each event.
      // It would probably be better to move the ProcessHistory
      // construction out to somewhere which persists for longer than one
      // Event.
      // Note: threading: We must protect processHistoryPtr_!  The id() call can modify it!
      ProcessHistoryRegistry::emplace(processHistoryPtr_->id(), *processHistoryPtr_);
      // Note: threading: We must protect processHistoryPtr_!  The id() call can modify it!
      // Note: threading: We are modifying Run, SubRun, Event, and Results principals here, and their *Auxiliary
      // Note: threading: and the event principal art::History.
      //setProcessHistoryID(processHistoryPtr_->id());
      setProcessHistoryIDcombined(processHistoryPtr_->id());
    }
  }

  GroupQueryResult
  Principal::
  getBySelector(TypeID const& productType, SelectorBase const& sel) const
  {
    std::vector<GroupQueryResult> results;
    int nFound = findGroupsForProduct(false, productType, sel, results, true);
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
  getByLabel(TypeID const& productType, string const& label, string const& productInstanceName, string const& processName) const
  {
    std::vector<GroupQueryResult> results;
    Selector sel(ModuleLabelSelector{label} &&
                 ProductInstanceNameSelector{productInstanceName} &&
                 ProcessNameSelector{processName});
    int const nFound = findGroupsForProduct(false, productType, sel, results, true);
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
  getMany(TypeID const& productType, SelectorBase const& sel, std::vector<GroupQueryResult>& results) const
  {
    findGroupsForProduct(false, productType, sel, results, false);
  }

  size_t
  Principal::
  getMatchingSequence(TypeID const& elementType, SelectorBase const& selector, std::vector<GroupQueryResult>& results) const
  {
    return findGroupsForProduct(true, elementType, selector, results, true);
  }

  size_t
  Principal::
  findGroupsForProduct(bool const byElementType, TypeID const& wanted_product, SelectorBase const& sel,
                       std::vector<GroupQueryResult>& res, bool stopIfProcessHasMatch) const
  {
    size_t ret = 0;
    TClass* cl = nullptr;
    //
    // Cannot do this here because some tests expect to be able to
    // call here requesting a wanted_product that does not have
    // a dictionary and be ok because the productLookup fails.
    // See issue #8532.
    //if (!byElementType) {
    //  cl = TClass::GetClass(wrappedClassName(wanted_product.className()).c_str());
    //  if (!cl) {
    //    throw Exception(errors::DictionaryNotFound)
    //        << "Dictionary not found for "
    //        << wrappedClassName(wanted_product.className())
    //        << ".\n";
    //  }
    //}
    auto findGroups = [this, &sel, &res, stopIfProcessHasMatch](Principal const& principal, ProcessLookup const& pl,
                                                                TypeID wanted_wrapper/*=TypeID()*/) {
      // Loop over processes in reverse time order.  Sometimes we want to stop
      // after we find a process with matches so check for that at each step.
      // Note: threading: We must protect the process history iterators here
      // Note: threading: against possible invalidation by output modules inserting
      // Note: threading: a process history entry while we are iterating.
      auto findGroupForProcess = [this, &sel, &res, &wanted_wrapper, &principal](auto const& pid) {
        auto try_group = [this, &sel, &res, &wanted_wrapper](auto& group) {
          if (!group) {
            //cout << "-----> Principal::try_group: invalid group" << endl;
            return false;
          }
          if (!sel.match(group->productDescription())) {
            //cout << "-----> Principal::try_group: no selector match" << endl;
            return false;
          }
          //cout << "-----> Principal::try_group: resolving" << endl;
          if (!group->tryToResolveProduct(wanted_wrapper)) {
            //cout << "-----> Principal::try_group: not found" << endl;
            return false;
          }
          //cout << "-----> Principal::try_group: found" << endl;
          // Found a good match, save it.
          res.emplace_back(group);
          return true;
        };
        cet::exempt_ptr<Group> group;
        // Look for a match in the primary principal.
        //cout << "-----> Principal::findGroupsForProcess: wanted: " << wanted_wrapper.name()
        //     << " search primay for pid: " << pid.value() << endl;
        auto PI = principal.groups_.find(pid);
        if (PI != principal.groups_.end()) {
          group = PI->second.get();
          auto found = try_group(group);
          if (found) {
            //cout << "-----> Principal::findGroupsForProcess: found" << endl;
            return;
          }
        }
        // Was not found in the primary principal, try all the secondaries until found.
        for (auto& p : principal.secondaryPrincipals_) {
          auto I = p->groups_.find(pid);
          if (I == p->groups_.end()) {
            continue;
          }
          group = I->second.get();
          auto found = try_group(group);
          if (found) {
            return;
          }
        }
      };
      struct ReversedProcessHistory {
        ProcessHistory& ph_;
        ReversedProcessHistory(ProcessHistory& ph) : ph_(ph) {}
        auto begin() { return ph_.rbegin(); }
        auto end() { return ph_.rend(); }
      };
      // Note: threading: Take a lock to prevent invalidation of the ProcessHistory iterator.
      lock_guard<recursive_mutex> sentry(principal.processHistoryPtr_->get_mutex());
      for (auto const& procConf : ReversedProcessHistory(*principal.processHistoryPtr_)) {
        if (!res.empty() && stopIfProcessHasMatch) {
          return res.size();
        }
        //cout << "-----> Principal::findGroups: wanted: " << wanted_wrapper.name()
        //     << " search process: " << procConf.processName() << endl;
        auto J = pl.find(procConf.processName());
        if (J == pl.end()) {
          continue;
        }
        //cout << "-----> Principal::findGroups: found" << endl;
        for (auto const& pid : J->second) {
          findGroupForProcess(pid);
        }
      }
      return res.size();
    };
    for (auto const& pl : (byElementType ? ProductMetaData::instance().elementLookup() :
                           ProductMetaData::instance().productLookup())) {
      auto I = pl[branchType_].find(wanted_product.friendlyClassName());
      if (I == pl[branchType_].end()) {
        continue;
      }
      if (byElementType) {
        ret = findGroups(*this, I->second, TypeID());
      }
      else {
        // Note: threading: We are risking deadlock here if the user gives us a
        // Note: threading: wanted_product that we have not already fetched from
        // Note: threading: the root type system before going multithreaded.
        cl = TClass::GetClass(wrappedClassName(wanted_product.className()).c_str());
        if (!cl) {
          throw Exception(errors::DictionaryNotFound)
            << "Dictionary not found for "
            << wrappedClassName(wanted_product.className())
            << ".\n";
        }
        ret = findGroups(*this, I->second, TypeID(cl->GetTypeInfo()));
      }
      if (ret) {
        return ret;
      }
    }
    while (true) {
      int const err = delayedReader_->openNextSecondaryFile(nextSecondaryFileIdx_);
      if (err != -2) {
        // there are more files to try
        ++nextSecondaryFileIdx_;
      }
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
      auto const& pl = (byElementType) ?  ProductMetaData::instance().elementLookup()[nextSecondaryFileIdx_] :
        ProductMetaData::instance().productLookup()[nextSecondaryFileIdx_];
      auto I = pl[branchType_].find(wanted_product.friendlyClassName());
      if (I == pl[branchType_].end()) {
        continue;
      }
      if (byElementType) {
        ret = findGroups(*secondaryPrincipals_.back().get(), I->second, TypeID());
      }
      else {
        cl = TClass::GetClass(wrappedClassName(wanted_product.className()).c_str());
        if (!cl) {
          throw Exception(errors::DictionaryNotFound)
            << "Dictionary not found for "
            << wrappedClassName(wanted_product.className())
            << ".\n";
        }
        ret = findGroups(*secondaryPrincipals_.back().get(), I->second, TypeID(cl->GetTypeInfo()));
      }
      if (ret) {
        return ret;
      }
    }
    return 0;
  }

  // Used by run, subrun, event, and results principals to validate puts.
  cet::exempt_ptr<Group> const
  Principal::
  getGroupLocal(ProductID const pid) const
  {
    auto I = groups_.find(pid);
    if (I != groups_.end()) {
      return I->second.get();
    }
    return nullptr;
  }

  // Used by art::DataViewImpl<T>::get(ProductID const pid, Handle<T>& result) const. (easy user-facing api)
  // Used by Principal::productGetter(ProductID const pid) const
  //   Used by (Run,SubRun,Event,Results)::productGetter (advanced user-facing api)
  GroupQueryResult
  Principal::
  getByProductID(ProductID const& pid) const
  {
    if (auto const g = getGroupTryAllFiles(pid)) {
      return GroupQueryResult{g};
    }
    auto whyFailed = make_shared<Exception>(errors::ProductNotFound, "InvalidID");
    *whyFailed << "Principal::getByProductID: no product with branch type: " << branchType_ << " product id: " << pid << "\n";
    return GroupQueryResult{whyFailed};
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
  Principal::
  getForOutput(ProductID const& pid, bool resolveProd) const
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
    auto const& pmd = ProductMetaData::instance();
    auto const bt = g->productDescription().branchType();
    if (
        resolveProd && // asked to resolve
        !g->anyProduct()->isPresent() && // wrapper says it is a dummy, and
        (
         (pmd.presentWithFileIdx(bt, pid) != MasterProductRegistry::DROPPED) || // product is in the input, or
         pmd.produced(bt, pid) // is a produced product
         ) && // , and
        (bt == InEvent) && // it is an event product, and FIXME: why are run, subrun, and results products not checked?
        productstatus::present(g->productProvenance()->productStatus()) // provenance says present
        ) {
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

  // Used by Principal::getByProductID(ProductID const& pid) const
  //   Used by art::DataViewImpl<T>::get(ProductID const pid, Handle<T>& result) const. (easy user-facing api)
  //   Used by Principal::productGetter(ProductID const pid) const
  //     Used by (Run,SubRun,Event,Results)::productGetter (advanced user-facing api)
  // Used by Principal::getForOutput(ProductID const& pid, bool resolveProd) const
  //   Used by RootOutputFile to fetch products being written to disk.
  //   Used by FileDumperOutput_module.
  //   Used by ProvenanceCheckerOutput_module.
  cet::exempt_ptr<Group const> const
  Principal::
  getGroupTryAllFiles(ProductID const& pid) const
  {
    cet::exempt_ptr<Group> ret;
    auto const index = ProductMetaData::instance().presentWithFileIdx(branchType_, pid);
    bool const produced = ProductMetaData::instance().produced(branchType_, pid);
    if (produced || (index == 0)) {
      // Note: Produced products are in the primary principal.
      auto it = groups_.find(pid);
      if (it != groups_.end()) {
        ret = it->second.get();
      }
      return ret;
    }
    if ((index > 0) && ((index - 1) < secondaryPrincipals_.size())) {
      // Product is in one of the files already opened.
      auto const& groups = secondaryPrincipals_[index-1]->groups_;
      auto it = groups.find(pid);
      if (it != groups.end()) {
        ret = it->second.get();
      }
      return ret;
    }
    // Not in any open file, try the secondary files in order.
    while (1) {
      int err = delayedReader_->openNextSecondaryFile(nextSecondaryFileIdx_);
      if (err != -2) {
        // there are more files to try
        ++nextSecondaryFileIdx_;
      }
      if (err == -2) {
        // No more files.
        break;
      }
      if (err == -1) {
        // Run, SubRun, or Event not found.
        continue;
      }
      auto const index = ProductMetaData::instance().presentWithFileIdx(branchType_, pid);
      if (index == MasterProductRegistry::DROPPED) {
        continue;
      }
      auto const& p = secondaryPrincipals_[index-1];
      auto iter = p->groups_.find(pid);
      if (iter != p->groups_.end()) {
        ret = iter->second.get();
        return ret;
      }
      break;
    }
    return ret;
  }

  BranchType
  Principal::
  branchType() const
  {
    return branchType_;
  }

  // Used by RootOutputFile
  // Used by Run
  RunAuxiliary const&
  Principal::
  runAux() const
  {
    return runAux_;
  }

  SubRunAuxiliary const&
  Principal::
  subRunAux() const
  {
    return subRunAux_;
  }

  EventAuxiliary const&
  Principal::
  eventAux() const
  {
    return eventAux_;
  }

  ResultsAuxiliary const&
  Principal::
  resultsAux() const
  {
    return resultsAux_;
  }

  // Used by EDFilter
  // Used by EDProducer
  // Used by EventProcessor
  // Used by RootInputFile
  // Used by RootOutput_module
  RunID const&
  Principal::
  runID() const
  {
    return runAux_.id();
  }

  SubRunID
  Principal::
  subRunID() const
  {
    return subRunAux_.id();
  }

  EventID const&
  Principal::
  eventID() const
  {
    return eventAux_.id();
  }

  // Used by test -- art/art/test/Integration/ToySource.cc
  // Used by test -- art/art/test/Integration/GeneratorTest_source.cc
  RunNumber_t
  Principal::
  run() const
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
  Principal::
  subRun() const
  {
    if (branchType_ == InSubRun) {
      return subRunAux_.subRun();
    }
    return eventAux_.id().subRun();
  }

  EventNumber_t
  Principal::
  event() const
  {
    return eventAux_.id().event();
  }

  Timestamp const&
  Principal::
  beginTime() const
  {
    if (branchType_ == InRun) {
      return runAux_.beginTime();
    }
    return subRunAux_.beginTime();
  }

  // Used by EventProcessor
  Timestamp const&
  Principal::
  endTime() const
  {
    if (branchType_ == InRun) {
      return runAux_.endTime();
    }
    return subRunAux_.endTime();
  }

  void
  Principal::
  endTime(Timestamp const& time)
  {
    if (branchType_ == InRun) {
      runAux_.endTime(time);
      return;
    }
    subRunAux_.setEndTime(time);
  }

  Timestamp const&
  Principal::
  time() const
  {
    return eventAux_.time();
  }

  RangeSet
  Principal::
  seenRanges() const
  {
    return rangeSet_;
  }

  void
  Principal::
  updateSeenRanges(RangeSet const& rs)
  {
    rangeSet_ = rs;
  }

  RunPrincipal&
  Principal::
  runPrincipal() const
  {
    if (!runPrincipal_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL runPrincipal.\n";
    }
    return *runPrincipal_;
  }

  SubRunPrincipal&
  Principal::
  subRunPrincipal() const
  {
    if (!subRunPrincipal_) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL subRunPrincipal.\n";
    }
    return *subRunPrincipal_;
  }

  cet::exempt_ptr<RunPrincipal>
  Principal::
  runPrincipalExemptPtr() const
  {
    return runPrincipal_;
  }

  cet::exempt_ptr<SubRunPrincipal>
  Principal::
  subRunPrincipalExemptPtr() const
  {
    return subRunPrincipal_;
  }

  void
  Principal::
  setRunPrincipal(cet::exempt_ptr<RunPrincipal> rp)
  {
    runPrincipal_ = rp;
  }

  void
  Principal::
  setSubRunPrincipal(cet::exempt_ptr<SubRunPrincipal> srp)
  {
    subRunPrincipal_ = srp;
  }

  bool
  Principal::
  isReal() const
  {
    return eventAux_.isRealData();
  }

  EventAuxiliary::ExperimentType
  Principal::
  ExperimentType() const
  {
    return eventAux_.experimentType();
  }

  History const&
  Principal::
  history() const
  {
    return *history_;
  }

  EventSelectionIDVector const&
  Principal::
  eventSelectionIDs() const
  {
    return history_->eventSelectionIDs();
  }

  bool
  Principal::
  isLastInSubRun() const
  {
    return lastInSubRun_;
  }

  void
  Principal::
  put(BranchDescription const& bd, unique_ptr<ProductProvenance const>&& pp, unique_ptr<EDProduct>&& edp, unique_ptr<RangeSet>&& rs)
  {
    assert(edp);
    if (!bd.produced()) {
      throw art::Exception(art::errors::ProductRegistrationFailure, "Principal::put:")
        << "Problem found during put of "
        << branchType_
        << " product: attempt to put a product not declared as produced for "
        << bd.branchName()
        << '\n';
    }
    if ((branchType_ == InRun) || (branchType_ == InSubRun)) {
      // Note: We intentionally allow group and provenance replacement for run and subrun products.
      auto group = getGroupLocal(bd.productID());
      //insert_pp(std::move(pp));
      //group->setProduct(move(edp), move(rs));
      group->setProductAndProvenance(move(pp), move(edp), move(rs));
    }
    else {
      auto group = getGroupLocal(bd.productID());
      if (group.get() != nullptr) {
        if (group->anyProduct() != nullptr) {
          throw art::Exception(art::errors::ProductRegistrationFailure, "Principal::put:")
            << "Problem found during put of "
            << branchType_
            << " product: product already put for "
            << bd.branchName()
            << '\n';
        }
      }
      //insert_pp(std::move(pp));
      //group->setProduct(move(edp), RangeSet::invalid());
      group->setProductAndProvenance(move(pp), move(edp), move(make_unique<RangeSet>()));
    }
  }

} // namespace art
