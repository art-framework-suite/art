#ifndef art_Framework_Principal_Principal_h
#define art_Framework_Principal_Principal_h
// vim: set sw=2 expandtab :

//  Principal
//
//  Pure abstract base class for Run-, SubRun-, and EventPrincipal,
//  the classes which manage data products.
//
//  The major internal component is the Group, which contains an EDProduct
//  and its associated Provenance, along with ancillary transient information
//  regarding the two.  Groups are handled through shared pointers.
//
//  The Principal returns GroupQueryResult, rather than a shared
//  pointer to a Group, when queried.

#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/OutputHandle.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/detail/type_aliases.h"
#include "canvas/Persistency/Common/PrincipalBase.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventRange.h"
#include "canvas/Persistency/Provenance/History.h"
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
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/value_ptr.h"
#include "tbb/concurrent_unordered_map.h"

#include <atomic>
#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace art {

class EDProduct;
class EmptyEvent;
class OutputModule;
class RootDelayedReader;
class RootOutput;
class SourceHelper;

class Principal : public PrincipalBase {

  // Let RootDelayedReader replace Run and SubRun product provenances.
  friend class RootDelayedReader;

public: // TYPES

  //using GroupCollection = std::map<ProductID, std::unique_ptr<Group>>;
  using GroupCollection = tbb::concurrent_unordered_map<ProductID, std::unique_ptr<Group>>;
  using const_iterator = GroupCollection::const_iterator;

public: // MEMBER FUNCTIONS -- Special Member Functions

  virtual
  ~Principal() noexcept;

  Principal(BranchType, ProcessConfiguration const&, ProcessHistoryID const&,
            std::unique_ptr<DelayedReader>&&);

  // Run
  Principal(RunAuxiliary const&, ProcessConfiguration const&,
            std::unique_ptr<DelayedReader>&& reader = std::make_unique<NoDelayedReader>());

  // SubRun
  Principal(SubRunAuxiliary const&, ProcessConfiguration const&,
            std::unique_ptr<DelayedReader>&& reader = std::make_unique<NoDelayedReader>());

  // Event
  Principal(EventAuxiliary const&, ProcessConfiguration const&,
            std::unique_ptr<History>&& history = std::make_unique<History>(),
            std::unique_ptr<DelayedReader>&& reader = std::make_unique<NoDelayedReader>(),
            bool lastInSubRun = false);

  // Results
  Principal(ResultsAuxiliary const&, ProcessConfiguration const&,
            std::unique_ptr<DelayedReader>&& reader = std::make_unique<NoDelayedReader>());

  Principal(Principal const&) = delete;

  Principal(Principal&&) = delete;

  Principal&
  operator=(Principal const&) = delete;

  Principal&
  operator=(Principal&&) = delete;

public: // MEMBER FUNCTIONS -- Interface for DataViewImpl<T>

  // Used by art::DataViewImpl<T>::get(ProductID const pid, Handle<T>& result) const. (easy user-facing api)
  // Used by Principal::productGetter(ProductID const pid) const
  //   Used by (Run,SubRun,Event,Results)::productGetter (advanced user-facing api)
  GroupQueryResult
  getByProductID(ProductID const&) const;

  GroupQueryResult
  getBySelector(TypeID const&, SelectorBase const&) const;

  GroupQueryResult
  getByLabel(TypeID const&, std::string const& label, std::string const& productInstanceName, std::string const& processName) const;

  void
  getMany(TypeID const&, SelectorBase const&, std::vector<GroupQueryResult>& results) const;

  // Used only by DataViewImpl<T> to implement getView.
  // Return a vector of GroupQueryResult to products which are sequences, have a nested type named 'value_type', and
  // where elementType the same as, or a public base of, this value_type, and which match the given selector.
  size_t
  getMatchingSequence(TypeID const& elementType, SelectorBase const&, std::vector<GroupQueryResult>&) const;

  // Note: Used only by DataViewImpl::ProductGetter!
  // Note: LArSoft uses this extensively to create a Ptr by hand.
  EDProductGetter const*
  productGetter(ProductID const& pid) const;

  ProcessHistory const&
  processHistory() const;

  // This is intended to be used by a module that fetches a very
  // large data product, makes a copy, and would like to release
  // the memory held by the original immediately.
  void
  removeCachedProduct(ProductID const&) const;

public: // MEMBER FUNCTIONS -- Interface for other parts of art

  // Used by RootOutputFile to fetch products being written to disk.
  // Used by FileDumperOutput_module.
  // Used by ProvenanceCheckerOutput_module.
  // We invoke the delay reader now if no user module has ever fetched them
  // for this principal if resolvedProd is true.
  // Note: This attempts to resolved the product and converts
  // Note: the resulting group into an OutputHandle.
  OutputHandle
  getForOutput(ProductID const&, bool resolveProd) const;

  // Used only by RootInputFile::Read(Run,SubRun,Event)ForSecondaryFile
  void
  addSecondaryPrincipal(std::unique_ptr<Principal>&&);

  // Used only by RootInputFile to implement the delayedRead*Products config options.
  // Read all data products and provenance immediately, if available.
  void
  readImmediate() const;

  // Used only by get_ProductDescription.
  ProcessConfiguration const&
  processConfiguration() const;

  // Used by Group
  // Used by RootOutputFile
  // Used by ProvenanceCheckerOutput_module
  // What used to be the functionality of BranchMapper.
  cet::exempt_ptr<ProductProvenance const>
  branchToProductProvenance(ProductID const&) const;

  // Used by FileDumperOutput_module
  // Used by DataViewImpl
  size_t
  size() const;

  // Note: Used only by OutputModule::updateBranchChildren and some dumper/checker output modules.
  const_iterator
  begin() const;

  //const_iterator
  //cbegin() const;

  // Note: Used only by OutputModule::updateBranchChildren and some dumper/checker output modules.
  const_iterator
  end() const;

  //const_iterator
  //cend() const;

  // Used by (Run, SubRun, Event)Principal
  // Used by RootOutput_module (for ResultsPrincipal, and drop on output)
  // Flag that we have been updated in the current process.
  void
  addToProcessHistory();

  // Used by FileDumperOutput_module
  // Obtain the branch type suitable for products inserted into the principal.
  BranchType
  branchType() const;

  // Used by EDProducer
  // Used by EDFilter
  RangeSet
  seenRanges() const;

  void
  put(BranchDescription const&, std::unique_ptr<ProductProvenance const>&&, std::unique_ptr<EDProduct>&&,
      std::unique_ptr<RangeSet>&&);

public: // MEMBER FUNCTIONS -- Used to be in subclasses

  // Used by RootOutputFile
  // Used by Run
  RunAuxiliary const&
  runAux() const;

  SubRunAuxiliary const&
  subRunAux() const;

  EventAuxiliary const&
  eventAux() const;

  ResultsAuxiliary const&
  resultsAux() const;

  // Used by EDFilter
  // Used by EDProducer
  // Used by EventProcessor
  // Used by RootInputFile
  // Used by RootOutput_module
  RunID const&
  runID() const;

  SubRunID
  subRunID() const;

  EventID const&
  eventID() const;

  // Used by test -- art/art/test/Integration/ToySource.cc
  // Used by test -- art/art/test/Integration/GeneratorTest_source.cc
  RunNumber_t
  run() const;

  SubRunNumber_t
  subRun() const;

  EventNumber_t
  event() const;

  Timestamp const&
  beginTime() const;

  // Used by EventProcessor
  Timestamp const&
  endTime() const;

  void
  endTime(Timestamp const& time);

  Timestamp const&
  time() const;

  // Used by EndPathExecutor
  void
  updateSeenRanges(RangeSet const& rs);

  RunPrincipal&
  runPrincipal() const;

  SubRunPrincipal&
  subRunPrincipal() const;

  cet::exempt_ptr<RunPrincipal>
  runPrincipalExemptPtr() const;

  cet::exempt_ptr<SubRunPrincipal>
  subRunPrincipalExemptPtr() const;

  void
  setRunPrincipal(cet::exempt_ptr<RunPrincipal> rp);

  void
  setSubRunPrincipal(cet::exempt_ptr<SubRunPrincipal> srp);

  EventAuxiliary::ExperimentType
  ExperimentType() const;

  bool
  isReal() const;

  EventSelectionIDVector const&
  eventSelectionIDs() const;

  History const&
  history() const;

  bool
  isLastInSubRun() const;

private: // MEMBER FUNCTIONS

  // Used by our ctors.
  void
  ctor_create_groups();

  // Used by our ctors.
  void
  ctor_read_provenance();

  // Used by our ctors.
  void
  ctor_fetch_process_history(ProcessHistoryID const&);

  // Used by our ctors.
  void
  ctor_add_to_process_history();

  // Used by our ctors.
  // Used by insert_pp.
  // Used by branchToProductProvenance.
  // Used by put.
  cet::exempt_ptr<Group> const
  getGroupLocal(ProductID const) const;

  // Used by RootDelayedReader to insert the data product provenance.
  void
  insert_pp(std::unique_ptr<ProductProvenance const>&&);

  // Implementation of the DataViewImpl<T> API.
  size_t
  findGroupsForProduct(bool const byElementType, TypeID const& wanted_product, SelectorBase const&,
                       std::vector<GroupQueryResult>&, bool stopIfProcessHasMatch) const;

  // Note: Used only by canvas RefCoreStreamer.cc through PrincipalBase::getEDProductGetter!
  virtual
  EDProductGetter const*
  getEDProductGetter_(ProductID const&) const override;

  // Used by Principal::getByProductID(ProductID const& pid) const
  //   Used by art::DataViewImpl<T>::get(ProductID const pid, Handle<T>& result) const. (easy user-facing api)
  //   Used by Principal::productGetter(ProductID const pid) const
  //     Used by (Run,SubRun,Event,Results)::productGetter (advanced user-facing api)
  // Used by Principal::getForOutput(ProductID const pid, bool resolveProd) const
  //   Used by RootOutputFile to fetch products being written to disk.
  //   Used by FileDumperOutput_module.
  //   Used by ProvenanceCheckerOutput_module.
  cet::exempt_ptr<Group const> const
  getGroupTryAllFiles(ProductID const&) const;

protected: // MEMBER DATA -- For used by derived types

  BranchType
  branchType_{};

  // Used to deal with TriggerResults.
  void
  fillGroup(BranchDescription const&);

  // Used by addToProcessHistory()
  void
  setProcessHistoryIDcombined(ProcessHistoryID const&);

private: // MEMBER DATA -- Mine, all mine!

  std::shared_ptr<ProcessHistory>
  processHistoryPtr_{std::make_shared<ProcessHistory>()};

  mutable
  std::atomic<bool>
  processHistoryModified_{false};

  ProcessConfiguration const&
  processConfiguration_;

  tbb::concurrent_unordered_map<ProductID, std::unique_ptr<Group>>
  groups_{};

  // Pointer to the reader that will be used to obtain
  // EDProducts from the persistent store.
  std::unique_ptr<DelayedReader>
  delayedReader_{nullptr};

  // Secondary principals.  Note that the lifetime of run
  // and subRun principals is the lifetime of the input file,
  // while the lifetime of event principals ends at the next
  // event read.
  std::vector<std::unique_ptr<Principal>>
  secondaryPrincipals_{};

  // Index into the secondary file names vector of the next
  // file that a secondary principal should be created from.
  mutable
  int
  nextSecondaryFileIdx_{};

  RangeSet
  rangeSet_{RangeSet::invalid()};

  RunAuxiliary
  runAux_{};

  SubRunAuxiliary
  subRunAux_{};

  EventAuxiliary
  eventAux_{};

  ResultsAuxiliary
  resultsAux_{};

  cet::exempt_ptr<RunPrincipal>
  runPrincipal_{nullptr};

  cet::exempt_ptr<SubRunPrincipal>
  subRunPrincipal_{nullptr};

  std::unique_ptr<History>
  history_{};

  bool lastInSubRun_{false};

};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_Principal_h */
