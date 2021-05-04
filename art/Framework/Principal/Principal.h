#ifndef art_Framework_Principal_Principal_h
#define art_Framework_Principal_Principal_h
// vim: set sw=2 expandtab :

// =================================================================
// Principal
//
// Pure abstract base class for Run-, SubRun-, and EventPrincipal,
// the classes which manage data products.
//
// The major internal component is the Group, which contains an
// EDProduct and its associated Provenance, along with ancillary
// transient information regarding the two.  Groups are handled
// through shared pointers.
//
// The Principal returns GroupQueryResult, rather than a shared
// pointer to a Group, when queried.
// =================================================================

#include "art/Framework/Principal/DelayedReader.h"
#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/OutputHandle.h"
#include "art/Framework/Principal/ProcessTag.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/ModuleContext.h"
#include "canvas/Persistency/Common/PrincipalBase.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventRange.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/ResultsAuxiliary.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Utilities/TypeID.h"
#include "canvas/Utilities/WrappedTypeID.h"
#include "cetlib/exempt_ptr.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace art {

  class EDProduct;

  class Principal : public PrincipalBase {
  public:
    using GroupCollection = std::map<ProductID, std::unique_ptr<Group>>;
    using const_iterator = GroupCollection::const_iterator;
    enum class allowed_processes { current_process, input_source, all };

    // The destructor is defined in the header so that overrides of
    // DelayedReader's readFromSecondaryFile_ virtual function can
    // return an std::unique_ptr<Principal> object (std::unique_ptr
    // instantiations require a well-formed deleter).
    virtual ~Principal() noexcept
    {
      presentProducts_ = nullptr;
      producedProducts_ = nullptr;
      delete eventAux_.load();
    }

    Principal(BranchType,
              ProcessConfiguration const&,
              cet::exempt_ptr<ProductTable const> presentProducts,
              ProcessHistoryID const&,
              std::unique_ptr<DelayedReader>&&);

    // Run
    Principal(RunAuxiliary const&,
              ProcessConfiguration const&,
              cet::exempt_ptr<ProductTable const> presentProducts,
              std::unique_ptr<DelayedReader>&& reader =
                std::make_unique<NoDelayedReader>());

    // SubRun
    Principal(SubRunAuxiliary const&,
              ProcessConfiguration const&,
              cet::exempt_ptr<ProductTable const> presentProducts,
              std::unique_ptr<DelayedReader>&& reader =
                std::make_unique<NoDelayedReader>());

    // Event
    Principal(EventAuxiliary const&,
              ProcessConfiguration const&,
              cet::exempt_ptr<ProductTable const> presentProducts,
              std::unique_ptr<History>&& history = std::make_unique<History>(),
              std::unique_ptr<DelayedReader>&& reader =
                std::make_unique<NoDelayedReader>(),
              bool lastInSubRun = false);

    // Results
    Principal(ResultsAuxiliary const&,
              ProcessConfiguration const&,
              cet::exempt_ptr<ProductTable const> presentProducts,
              std::unique_ptr<DelayedReader>&& reader =
                std::make_unique<NoDelayedReader>());

    // Disable copying and moving.
    Principal(Principal const&) = delete;
    Principal& operator=(Principal const&) = delete;

    // Interface for DataViewImpl<T>
    //
    // - Used by art::DataViewImpl<T>::get(ProductID const pid,
    //   Handle<T>& result) const. (easy user-facing api) Used by
    //   Principal::productGetter(ProductID const pid) const
    //
    // - Used by (Run,SubRun,Event,Results)::productGetter (advanced
    //   user-facing api)
    GroupQueryResult getByProductID(ProductID const pid) const;

    GroupQueryResult getBySelector(ModuleContext const& mc,
                                   WrappedTypeID const& wrapped,
                                   SelectorBase const&,
                                   ProcessTag const&) const;
    GroupQueryResult getByLabel(ModuleContext const& mc,
                                WrappedTypeID const& wrapped,
                                std::string const& label,
                                std::string const& productInstanceName,
                                ProcessTag const& processTag) const;
    std::vector<GroupQueryResult> getMany(ModuleContext const& mc,
                                          WrappedTypeID const& wrapped,
                                          SelectorBase const&,
                                          ProcessTag const&) const;

    std::vector<InputTag> getInputTags(ModuleContext const& mc,
                                       WrappedTypeID const& wrapped,
                                       SelectorBase const&,
                                       ProcessTag const&) const;

    // Used only by DataViewImpl<T> to implement getView.
    // FIXME: Return a vector of GroupQueryResult to products which
    //        are sequences, have a nested type named 'value_type',
    //        and where elementType the same as, or a public base of,
    //        this value_type, and which match the given selector.
    std::vector<cet::exempt_ptr<Group>> getMatchingSequence(
      ModuleContext const&,
      SelectorBase const&,
      ProcessTag const&) const;

    // Note: LArSoft uses this extensively to create a Ptr by hand.
    EDProductGetter const* productGetter(ProductID const& pid) const;

    ProcessHistory const& processHistory() const;

    // This is intended to be used by a module that fetches a very
    // large data product, makes a copy, and would like to release
    // the memory held by the original immediately.
    void removeCachedProduct(ProductID) const;

    // Interface for other parts of art

    // Note: We invoke the delay reader if no user module has fetched
    //       them for this principal if resolvedProd is true.  This
    //       attempts to resolved the product and converts the
    //       resulting group into an OutputHandle.
    OutputHandle getForOutput(ProductID const&, bool resolveProd) const;

    // Used to provide access to the product descriptions
    cet::exempt_ptr<BranchDescription const> getProductDescription(
      ProductID const pid,
      bool const alwaysEnableLookupOfProducedProducts = false) const;

    // The product tables data member for produced products is set by
    // the EventProcessor after the Principal is provided by the input
    // source.
    void createGroupsForProducedProducts(ProductTables const& producedProducts);
    void enableLookupOfProducedProducts(ProductTables const& producedProducts);

    // FIXME: This breaks the purpose of the
    //        Principal::addToProcessHistory() compare_exchange_strong
    //        because of the temporal hole between when the history is
    //        changed and when the flag is set, this must be fixed!
    void markProcessHistoryAsModified();

    // Read all data products and provenance immediately, if available.
    void readImmediate() const;

    ProcessConfiguration const& processConfiguration() const;

    cet::exempt_ptr<ProductProvenance const> branchToProductProvenance(
      ProductID const&) const;

    size_t size() const;

    const_iterator begin() const;
    const_iterator cbegin() const;

    const_iterator end() const;
    const_iterator cend() const;

    // Flag that we have been updated in the current process.
    void addToProcessHistory();

    BranchType branchType() const;

    RangeSet seenRanges() const;

    void put(BranchDescription const&,
             std::unique_ptr<ProductProvenance const>&&,
             std::unique_ptr<EDProduct>&&,
             std::unique_ptr<RangeSet>&&);

    // Used to be in subclasses
    RunAuxiliary const& runAux() const;
    SubRunAuxiliary const& subRunAux() const;
    EventAuxiliary const& eventAux() const;
    ResultsAuxiliary const& resultsAux() const;

    RunID const& runID() const;
    SubRunID subRunID() const;
    EventID const& eventID() const;

    RunNumber_t run() const;
    SubRunNumber_t subRun() const;
    EventNumber_t event() const;

    Timestamp const& beginTime() const;
    Timestamp const& endTime() const;

    void endTime(Timestamp const& time);
    Timestamp const& time() const;

    // Used by EndPathExecutor
    void updateSeenRanges(RangeSet const& rs);

    RunPrincipal const& runPrincipal() const;
    SubRunPrincipal const& subRunPrincipal() const;
    cet::exempt_ptr<RunPrincipal const> runPrincipalExemptPtr() const;
    SubRunPrincipal const* subRunPrincipalPtr() const;
    void setRunPrincipal(cet::exempt_ptr<RunPrincipal const> rp);
    void setSubRunPrincipal(cet::exempt_ptr<SubRunPrincipal const> srp);
    EventAuxiliary::ExperimentType ExperimentType() const;
    bool isReal() const;
    EventSelectionIDVector const& eventSelectionIDs() const;
    History const& history() const;
    bool isLastInSubRun() const;

  private:
    // Used by our ctors.
    void ctor_create_groups(cet::exempt_ptr<ProductTable const>);
    void ctor_read_provenance();
    void ctor_fetch_process_history(ProcessHistoryID const&);

    cet::exempt_ptr<Group> getGroupLocal(ProductID const) const;

    std::vector<cet::exempt_ptr<Group>> matchingSequenceFromInputFile(
      ModuleContext const&,
      SelectorBase const&) const;
    size_t findGroupsFromInputFile(
      ModuleContext const&,
      WrappedTypeID const& wrapped,
      SelectorBase const&,
      std::vector<cet::exempt_ptr<Group>>& results) const;
    size_t findGroups(ProcessLookup const&,
                      ModuleContext const&,
                      SelectorBase const&,
                      std::vector<cet::exempt_ptr<Group>>& groups) const;
    size_t findGroupsForProcess(
      std::vector<ProductID> const& vpid,
      ModuleContext const& mc,
      SelectorBase const& selector,
      std::vector<cet::exempt_ptr<Group>>& groups) const;
    bool producedInProcess(ProductID) const;
    bool presentFromSource(ProductID) const;
    auto tryNextSecondaryFile() const;

    // Implementation of the DataViewImpl API.
    std::vector<cet::exempt_ptr<Group>> findGroupsForProduct(
      ModuleContext const& mc,
      WrappedTypeID const& wrapped,
      SelectorBase const&,
      ProcessTag const&) const;

    EDProductGetter const* getEDProductGetter_(ProductID const&) const override;

    cet::exempt_ptr<Group const> getGroupTryAllFiles(ProductID const) const;

  protected:
    // Used to deal with TriggerResults.
    void fillGroup(BranchDescription const&);

    // Used by addToProcessHistory()
    void setProcessHistoryIDcombined(ProcessHistoryID const&);

  private:
    BranchType branchType_{};
    ProcessHistory processHistory_{};
    std::atomic<bool> processHistoryModified_;
    ProcessConfiguration const& processConfiguration_;

    // Product-lookup tables
    std::atomic<ProductTable const*> presentProducts_;
    std::atomic<ProductTable const*> producedProducts_;
    std::atomic<bool> enableLookupOfProducedProducts_;

    // Protects access to groups_.
    mutable std::recursive_mutex groupMutex_{};

    // All of the currently known data products.
    // tbb::concurrent_unordered_map<ProductID, std::unique_ptr<Group>>
    // groups_{};
    GroupCollection groups_{};

    // Pointer to the reader that will be used to obtain
    // EDProducts from the persistent store.
    std::unique_ptr<DelayedReader> delayedReader_{nullptr};

    // Secondary principals.  Note that the lifetimes of Results, Run,
    // and SubRun principals do not exceed the lifetime of the input
    // file.
    //
    // Note: To make secondary file-reading thread-safe, we will need to
    //       ensure that any adjustments to the secondaryPrincipals_
    //       vector is done atomically with respect to any reading of
    //       the vector.
    mutable std::vector<std::unique_ptr<Principal>> secondaryPrincipals_{};

    // Index into the secondary file names vector of the next
    // file that a secondary principal should be created from.
    mutable int nextSecondaryFileIdx_{};

    RangeSet rangeSet_{RangeSet::invalid()};
    RunAuxiliary runAux_{};
    SubRunAuxiliary subRunAux_{};

    // We own.
    std::atomic<EventAuxiliary*> eventAux_;

    ResultsAuxiliary resultsAux_{};
    cet::exempt_ptr<RunPrincipal const> runPrincipal_{nullptr};
    std::atomic<SubRunPrincipal const*> subRunPrincipal_;
    std::unique_ptr<History> history_{nullptr};
    bool lastInSubRun_{false};
  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_Principal_h */
