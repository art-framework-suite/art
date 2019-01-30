#include "art/Framework/Principal/Principal.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/OutputHandle.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/RangeSetsSupported.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Common/PrincipalBase.h"
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
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/ResultsAuxiliary.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "canvas/Utilities/WrappedTypeID.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "hep_concurrency/RecursiveMutex.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace cet;
using namespace hep::concurrency;
using namespace std;

namespace art {

  class EventPrincipal;

  namespace {

    template <typename T>
    class ReverseIteration;
    template <typename T>
    ReverseIteration<T> reverse_iteration(T const&);

    template <typename T>
    class ReverseIteration {
      friend ReverseIteration reverse_iteration<>(T const&);

    private:
      T const& t_;
      ReverseIteration(T const& t) : t_{t} {};

    public:
      auto
      begin() const
      {
        return crbegin(t_);
      }
      auto
      end() const
      {
        return crend(t_);
      }
    };

    template <typename T>
    ReverseIteration<T>
    reverse_iteration(T const& t)
    {
      return ReverseIteration<T>{t};
    }

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
      return make_unique<Group>(reader, bd, make_unique<RangeSet>(), gt);
    }

  } // unnamed namespace

  Principal::~Principal() noexcept
  {
    presentProducts_ = nullptr;
    producedProducts_ = nullptr;
    delete eventAux_.load();
    eventAux_.store(nullptr);
  }

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
                       std::unique_ptr<DelayedReader>&& reader)
    : branchType_{branchType}
    , processConfiguration_{pc}
    , delayedReader_{std::move(reader)}
  {
    processHistoryModified_ = false;
    presentProducts_ = presentProducts.get();
    producedProducts_ = nullptr;
    enableLookupOfProducedProducts_ = false;
    delayedReader_->setPrincipal(this);
    eventAux_ = nullptr;
    subRunPrincipal_ = nullptr;
    ctor_create_groups(presentProducts);
    ctor_read_provenance();
    ctor_fetch_process_history(hist);
  }

  // Run
  Principal::Principal(RunAuxiliary const& aux,
                       ProcessConfiguration const& pc,
                       cet::exempt_ptr<ProductTable const> presentProducts,
                       std::unique_ptr<DelayedReader>&&
                         reader /* = std::make_unique<NoDelayedReader>() */)
    : branchType_{InRun}
    , processConfiguration_{pc}
    , delayedReader_{std::move(reader)}
    , runAux_{aux}
  {
    processHistoryModified_ = false;
    presentProducts_ = presentProducts.get();
    producedProducts_ = nullptr;
    enableLookupOfProducedProducts_ = false;
    delayedReader_->setPrincipal(this);
    eventAux_ = nullptr;
    subRunPrincipal_ = nullptr;
    ctor_create_groups(presentProducts);
    ctor_read_provenance();
    ctor_fetch_process_history(runAux_.processHistoryID());
  }

  // SubRun
  Principal::Principal(SubRunAuxiliary const& aux,
                       ProcessConfiguration const& pc,
                       cet::exempt_ptr<ProductTable const> presentProducts,
                       std::unique_ptr<DelayedReader>&&
                         reader /* = std::make_unique<NoDelayedReader>() */)
    : branchType_{InSubRun}
    , processConfiguration_{pc}
    , delayedReader_{std::move(reader)}
    , subRunAux_{aux}
  {
    processHistoryModified_ = false;
    presentProducts_ = presentProducts.get();
    producedProducts_ = nullptr;
    enableLookupOfProducedProducts_ = false;
    delayedReader_->setPrincipal(this);
    eventAux_ = nullptr;
    subRunPrincipal_ = nullptr;
    ctor_create_groups(presentProducts);
    ctor_read_provenance();
    ctor_fetch_process_history(subRunAux_.processHistoryID());
  }

  // Event
  Principal::Principal(
    EventAuxiliary const& aux,
    ProcessConfiguration const& pc,
    cet::exempt_ptr<ProductTable const> presentProducts,
    std::unique_ptr<History>&& history /* = std::make_unique<History>() */,
    std::unique_ptr<DelayedReader>&&
      reader /* = std::make_unique<NoDelayedReader>() */,
    bool const lastInSubRun /* = false */)
    : branchType_{InEvent}
    , processConfiguration_{pc}
    , delayedReader_{std::move(reader)}
    , history_{move(history)}
    , lastInSubRun_{lastInSubRun}
  {
    processHistoryModified_ = false;
    presentProducts_ = presentProducts.get();
    producedProducts_ = nullptr;
    enableLookupOfProducedProducts_ = false;
    delayedReader_->setPrincipal(this);
    eventAux_ = new EventAuxiliary(aux);
    subRunPrincipal_ = nullptr;
    ctor_create_groups(presentProducts);
    ctor_read_provenance();
    ctor_fetch_process_history(history_->processHistoryID());
  }

  // Results
  Principal::Principal(ResultsAuxiliary const& aux,
                       ProcessConfiguration const& pc,
                       cet::exempt_ptr<ProductTable const> presentProducts,
                       std::unique_ptr<DelayedReader>&&
                         reader /* = std::make_unique<NoDelayedReader>() */)
    : branchType_{InResults}
    , processConfiguration_{pc}
    , delayedReader_{std::move(reader)}
    , resultsAux_{aux}
  {
    processHistoryModified_ = false;
    presentProducts_ = presentProducts.get();
    producedProducts_ = nullptr;
    enableLookupOfProducedProducts_ = false;
    delayedReader_->setPrincipal(this);
    eventAux_ = nullptr;
    subRunPrincipal_ = nullptr;
    ctor_create_groups(presentProducts);
    ctor_read_provenance();
    ctor_fetch_process_history(resultsAux_.processHistoryID());
  }

  void
  Principal::fillGroup(BranchDescription const& pd)
  {
    hep::concurrency::RecursiveMutexSentry sentry{groupMutex_, __func__};
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

    unique_ptr<Group> group = create_group(delayedReader_.get(), pd);
    groups_[pd.productID()] = move(group);
  }

  void
  Principal::setProcessHistoryIDcombined(ProcessHistoryID const& phid)
  {
    if (branchType_ == InRun) {
      runAux_.setProcessHistoryID(phid);
    } else if (branchType_ == InSubRun) {
      subRunAux_.setProcessHistoryID(phid);
    } else if (branchType_ == InEvent) {
      history_->setProcessHistoryID(phid);
    } else {
      resultsAux_.setProcessHistoryID(phid);
    }
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

  // Note: To make secondary file-reading thread-safe, we will need to
  //       ensure that any adjustments to the secondaryPrincipals_
  //       vector is done atomically with respect to any reading of
  //       the vector.
  void
  Principal::addSecondaryPrincipal(std::unique_ptr<Principal>&& val)
  {
    secondaryPrincipals_.emplace_back(std::move(val));
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
    for (auto const& pr : produced.descriptions) {
      auto const& pd = pr.second;
      assert(pd.branchType() == branchType_);
      // Create a group for the produced product.
      fillGroup(pd);
    }
  }

  void
  Principal::enableLookupOfProducedProducts(
    ProductTables const& /*producedProducts*/)
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
    hep::concurrency::RecursiveMutexSentry sentry{groupMutex_, __func__};
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
    hep::concurrency::RecursiveMutexSentry sentry{groupMutex_, __func__};
    return groups_.size();
  }

  Principal::const_iterator
  Principal::begin() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{groupMutex_, __func__};
    return groups_.begin();
  }

  Principal::const_iterator
  Principal::cbegin() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{groupMutex_, __func__};
    return groups_.cbegin();
  }

  Principal::const_iterator
  Principal::end() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{groupMutex_, __func__};
    return groups_.end();
  }

  Principal::const_iterator
  Principal::cend() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{groupMutex_, __func__};
    return groups_.cend();
  }

  // This is intended to be used by a module that fetches a very large
  // data product, makes a copy, and would like to release the memory
  // held by the original immediately.
  void
  Principal::removeCachedProduct(ProductID const pid) const
  {
    // MT-FIXME: May be called by a module task, need to protect the
    //           group with a lock.
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
      << "Attempt to remove unknown product corresponding to ProductID: " << pid
      << '\n'
      << "Please contact artists@fnal.gov\n";
  }

  // Used by Principal::put to insert the data product provenance.
  // Used by RootDelayedReader::getProduct_ to replace the product provenance
  // when merging run and subRun data products.
  void
  Principal::insert_pp(Group* grp,
                       std::unique_ptr<ProductProvenance const>&& pp)
  {
    grp->setProductProvenance(move(pp));
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
  //     Used by art::DataViewImpl<T>::get(ProductID const pid, Handle<T>&
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
  // the DataViewImpl::processHistory() interface to any module
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
      RecursiveMutexSentry sentry{processHistory_.get_mutex(), __func__};
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
      // MT note: We must protect processHistory_!  The id() call can
      //          modify it! Note: threading: We are modifying Run,
      //          SubRun, Event, and Results principals here, and
      //          their *Auxiliary Note: threading: and the event
      //          principal art::History.
      setProcessHistoryIDcombined(processHistory_.id());
    }
  }

  GroupQueryResult
  Principal::getBySelector(ModuleContext const& mc,
                           WrappedTypeID const& wrapped,
                           SelectorBase const& sel,
                           ProcessTag const& processTag) const
  {
    auto const& results =
      findGroupsForProduct(mc, wrapped, sel, processTag, true);
    if (results.empty()) {
      auto whyFailed =
        std::make_shared<art::Exception>(art::errors::ProductNotFound);
      *whyFailed << "getBySelector: Found zero products matching all criteria\n"
                 << "Looking for type: " << wrapped.product_type << "\n";
      return GroupQueryResult{whyFailed};
    }
    if (results.size() > 1) {
      throw art::Exception(art::errors::ProductNotFound)
        << "getBySelector: Found " << results.size()
        << " products rather than one which match all criteria\n"
        << "Looking for type: " << wrapped.product_type << "\n";
    }
    return results[0];
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
    auto const& results =
      findGroupsForProduct(mc, wrapped, sel, processTag, true);
    if (results.empty()) {
      auto whyFailed =
        std::make_shared<art::Exception>(art::errors::ProductNotFound);
      *whyFailed << "getByLabel: Found zero products matching all criteria\n"
                 << "Looking for type: " << wrapped.product_type << '\n'
                 << "Looking for module label: " << label << '\n'
                 << "Looking for productInstanceName: " << productInstanceName
                 << '\n';
      if (!processName.empty()) {
        *whyFailed << "Looking for process: " << processName << '\n';
      }
      return GroupQueryResult{whyFailed};
    }
    if (results.size() > 1) {
      Exception e{errors::ProductNotFound};
      e << "getByLabel: Found " << results.size()
        << " products rather than one which match all criteria\n"
        << "Looking for type: " << wrapped.product_type << '\n'
        << "Looking for module label: " << label << '\n'
        << "Looking for productInstanceName: " << productInstanceName << '\n';
      if (!processName.empty()) {
        e << "Looking for process: " << processName << '\n';
      }
      throw e;
    }
    return results[0];
  }

  Principal::GroupQueryResultVec
  Principal::getMany(ModuleContext const& mc,
                     WrappedTypeID const& wrapped,
                     SelectorBase const& sel,
                     ProcessTag const& processTag) const
  {
    return findGroupsForProduct(mc, wrapped, sel, processTag, false);
  }

  Principal::GroupQueryResultVec
  Principal::getMatchingSequence(ModuleContext const& mc,
                                 SelectorBase const& selector,
                                 ProcessTag const& processTag) const
  {
    GroupQueryResultVec results;
    // Find groups from current process
    if (processTag.current_process_search_allowed() &&
        enableLookupOfProducedProducts_.load()) {
      if (findGroups(producedProducts_.load()->viewLookup,
                     mc,
                     selector,
                     results,
                     true) != 0) {
        return results;
      }
    }

    if (!processTag.input_source_search_allowed()) {
      return results;
    }

    // Look through currently opened input files
    if (results.empty()) {
      results = matchingSequenceFromInputFile(mc, selector);
      if (!results.empty()) {
        return results;
      }
      for (auto const& sp : secondaryPrincipals_) {
        results = sp->matchingSequenceFromInputFile(mc, selector);
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
        results = new_sp->matchingSequenceFromInputFile(mc, selector);
        if (!results.empty()) {
          return results;
        }
      }
    }
    return results;
  }

  Principal::GroupQueryResultVec
  Principal::matchingSequenceFromInputFile(ModuleContext const& mc,
                                           SelectorBase const& selector) const
  {
    GroupQueryResultVec results;
    if (!presentProducts_.load()) {
      return results;
    }
    findGroups(
      presentProducts_.load()->viewLookup, mc, selector, results, true);
    return results;
  }

  std::size_t
  Principal::findGroups(ProcessLookup const& pl,
                        ModuleContext const& mc,
                        SelectorBase const& sel,
                        GroupQueryResultVec& res,
                        bool const stopIfProcessHasMatch,
                        TypeID const wanted_wrapper /*=TypeID()*/) const
  {
    // Loop over processes in reverse time order.  Sometimes we want
    // to stop after we find a process with matches so check for that
    // at each step.
    std::size_t found{};
    // MT note: We must protect the process history iterators here
    //          against possible invalidation by output modules
    //          inserting a process history entry while we are
    //          iterating.
    RecursiveMutexSentry sentry{processHistory_.get_mutex(), __func__};
    for (auto const& h : reverse_iteration(processHistory_)) {
      auto it = pl.find(h.processName());
      if (it != pl.end()) {
        found += findGroupsForProcess(it->second, mc, sel, res, wanted_wrapper);
      }
      if (stopIfProcessHasMatch && !res.empty()) {
        break;
      }
    }
    return found;
  }

  std::size_t
  Principal::findGroupsFromInputFile(ModuleContext const& mc,
                                     WrappedTypeID const& wrapped,
                                     SelectorBase const& selector,
                                     GroupQueryResultVec& results,
                                     bool const stopIfProcessHasMatch) const
  {
    if (!presentProducts_.load()) {
      return 0;
    }
    auto const& lookup = presentProducts_.load()->productLookup;
    auto it = lookup.find(wrapped.product_type.friendlyClassName());
    if (it == lookup.end()) {
      return 0;
    }
    return findGroups(it->second,
                      mc,
                      selector,
                      results,
                      stopIfProcessHasMatch,
                      wrapped.wrapped_product_type);
  }

  Principal::GroupQueryResultVec
  Principal::findGroupsForProduct(ModuleContext const& mc,
                                  WrappedTypeID const& wrapped,
                                  SelectorBase const& selector,
                                  ProcessTag const& processTag,
                                  bool const stopIfProcessHasMatch) const
  {
    GroupQueryResultVec results;
    unsigned ret{};
    // Find groups from current process
    if (processTag.current_process_search_allowed() &&
        enableLookupOfProducedProducts_.load()) {
      auto const& lookup = producedProducts_.load()->productLookup;
      auto it = lookup.find(wrapped.product_type.friendlyClassName());
      if (it != lookup.end()) {
        ret += findGroups(it->second,
                          mc,
                          selector,
                          results,
                          stopIfProcessHasMatch,
                          wrapped.wrapped_product_type);
      }
    }

    if (!processTag.input_source_search_allowed()) {
      return results;
    }

    // Look through currently opened input files
    ret += findGroupsFromInputFile(
      mc, wrapped, selector, results, stopIfProcessHasMatch);
    if (ret) {
      return results;
    }
    for (auto const& sp : secondaryPrincipals_) {
      if (sp->findGroupsFromInputFile(
            mc, wrapped, selector, results, stopIfProcessHasMatch)) {
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
      if (new_sp->findGroupsFromInputFile(
            mc, wrapped, selector, results, stopIfProcessHasMatch)) {
        return results;
      }
    }
    return results;
  }

  std::size_t
  Principal::findGroupsForProcess(std::vector<ProductID> const& vpid,
                                  ModuleContext const& mc,
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
    if (subRunPrincipal_.load() == nullptr) {
      throw Exception(errors::NullPointerError)
        << "Tried to obtain a NULL subRunPrincipal.\n";
    }
    return *subRunPrincipal_.load();
  }

  cet::exempt_ptr<RunPrincipal const>
  Principal::runPrincipalExemptPtr() const
  {
    return runPrincipal_;
  }

  SubRunPrincipal const*
  Principal::subRunPrincipalPtr() const
  {
    return subRunPrincipal_.load();
  }

  void
  Principal::setRunPrincipal(cet::exempt_ptr<RunPrincipal const> rp)
  {
    runPrincipal_ = rp;
  }

  void
  Principal::setSubRunPrincipal(cet::exempt_ptr<SubRunPrincipal const> srp)
  {
    subRunPrincipal_ = srp.get();
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
    return eventAux_.load()->isRealData();
  }

  EventAuxiliary::ExperimentType
  Principal::ExperimentType() const
  {
    return eventAux_.load()->experimentType();
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
        move(pp), move(edp), make_unique<RangeSet>());
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
    return *eventAux_.load();
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
    return eventAux_.load()->id();
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
    return eventAux_.load()->id().run();
  }

  SubRunNumber_t
  Principal::subRun() const
  {
    if (branchType_ == InSubRun) {
      return subRunAux_.subRun();
    }
    return eventAux_.load()->id().subRun();
  }

  EventNumber_t
  Principal::event() const
  {
    return eventAux_.load()->id().event();
  }

  Timestamp const&
  Principal::beginTime() const
  {
    if (branchType_ == InRun) {
      return runAux_.beginTime();
    }
    return subRunAux_.beginTime();
  }

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
    return eventAux_.load()->time();
  }

  int
  Principal::tryNextSecondaryFile() const
  {
    int const err =
      delayedReader_->openNextSecondaryFile(nextSecondaryFileIdx_);
    if (err != -2) {
      // there are more files to try
      ++nextSecondaryFileIdx_;
    }
    return err;
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
    if (auto const g = getGroupTryAllFiles(pid)) {
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
    hep::concurrency::RecursiveMutexSentry sentry{groupMutex_, __func__};
    auto it = groups_.find(pid);
    return it != groups_.cend() ? it->second.get() : nullptr;
  }

  cet::exempt_ptr<Group const>
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
