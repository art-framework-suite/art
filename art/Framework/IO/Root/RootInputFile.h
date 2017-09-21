#ifndef art_Framework_IO_Root_RootInputFile_h
#define art_Framework_IO_Root_RootInputFile_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/Inputfwd.h"
#include "art/Framework/IO/Root/RootInputFileSequence.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/Parentage.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductRegistry.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "canvas/Persistency/Provenance/ResultsAuxiliary.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/type_aliases.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/sqlite/Connection.h"

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

class TFile;
class TTree;
class TBranch;

namespace art {

namespace detail {

void
mergeAuxiliary(RunAuxiliary& left, RunAuxiliary const& right);

void
mergeAuxiliary(SubRunAuxiliary& left, SubRunAuxiliary const& right);

} // namespace detail

class DuplicateChecker;
class EventRangeHandler;
class GroupSelectorRules;
  class MasterProductRegistry;

class RootInputFile {

private: // TYPES

  class RootInputTree {

  public: // TYPES

    using BranchMap = input::BranchMap;
    using EntryNumber = input::EntryNumber;
    using EntryNumbers = input::EntryNumbers;

  public: // MEMBER FUNCTIONS -- Special Member Functions

    ~RootInputTree();

    RootInputTree(cet::exempt_ptr<TFile>, BranchType, int64_t saveMemoryObjectThreshold, cet::exempt_ptr<RootInputFile>, bool missingOK = false);

    RootInputTree(RootInputTree const&) = delete;

    RootInputTree(RootInputTree&&) = delete;

    RootInputTree&
    operator=(RootInputTree const&) = delete;

    RootInputTree&&
    operator=(RootInputTree&&) = delete;

  public: // MEMBER FUNCTIONS

    bool
    isValid() const;

    EntryNumber
    entries() const;

    TTree*
    tree() const;

    TTree*
    metaTree() const;

    TBranch*
    auxBranch() const;

    TBranch*
    productProvenanceBranch() const;

    BranchMap const&
    branches() const;

    void
    addBranch(BranchDescription const&);

    void
    dropBranch(std::string const& branchName);

  private: // MEMBER DATA

    TTree*
    tree_{nullptr};

    TTree*
    metaTree_{nullptr};

    TBranch*
    auxBranch_{nullptr};

    TBranch*
    productProvenanceBranch_{nullptr};

    EntryNumber
    entries_{0};

    BranchMap branches_{};
  };

public: // TYPES

  using RootInputTreePtrArray = std::array<std::unique_ptr<RootInputTree>, NumBranchTypes>;
  using EntryNumber = RootInputTree::EntryNumber;
  using EntryNumbers = RootInputTree::EntryNumbers;

public: // MEMBER FUNCTIONS -- Special Member Functions

  ~RootInputFile();

  RootInputFile(RootInputFile const&) = delete;

  RootInputFile(RootInputFile&&) = delete;

  RootInputFile&
  operator=(RootInputFile const&) = delete;

  RootInputFile&
  operator=(RootInputFile&&) = delete;

  RootInputFile(std::string const& fileName,
                std::string const& catalogName,
                ProcessConfiguration const& processConfiguration,
                std::string const& logicalFileName,
                std::unique_ptr<TFile>&& filePtr,
                EventID const& origEventID,
                unsigned int eventsToSkip,
                FastCloningInfoProvider const& fcip,
                unsigned int treeCacheSize,
                int64_t treeMaxVirtualSize,
                int64_t saveMemoryObjectThreashold,
                bool delayedReadEventProducts,
                bool delayedReadSubRunProducts,
                bool delayedReadRunProducts,
                InputSource::ProcessingMode processingMode,
                int forcedRunOffset,
                bool noEventSort,
                GroupSelectorRules const& groupSelectorRules,
                std::shared_ptr<DuplicateChecker> duplicateChecker,
                bool dropDescendantsOfDroppedProducts,
                bool readIncomingParameterSets,
                cet::exempt_ptr<RootInputFile> primaryFile,
                std::vector<std::string> const& secondaryFileNames,
                RootInputFileSequence* rifSequence,
                MasterProductRegistry& mpr);

public: // MEMBER FUNCTIONS

  void
  reportOpened();

  void
  close(bool reallyClose);

  std::unique_ptr<ResultsPrincipal>
  readResults();

  std::unique_ptr<RunPrincipal>
  readRun();

  std::unique_ptr<SubRunPrincipal>
  readSubRun(cet::exempt_ptr<RunPrincipal const>);

  std::unique_ptr<EventPrincipal>
  readEvent();

  bool
  readRunForSecondaryFile(RunID);

  bool
  readSubRunForSecondaryFile(SubRunID);

  bool
  readEventForSecondaryFile(EventID eID);

  std::string const&
  fileName() const;

  //RunAuxiliary&
  //runAux();

  //ResultsAuxiliary&
  //resultsAux();

  //SubRunAuxiliary&
  //subRunAux();

  //EventAuxiliary&
  //eventAux();

  RootInputTreePtrArray&
  treePointers();

  FileFormatVersion
  fileFormatVersion() const;

  bool
  fastClonable() const;

  std::unique_ptr<FileBlock>
  createFileBlock();

  bool
  setEntry_Event(EventID const& id, bool exact = true);

  bool
  setEntry_SubRun(SubRunID const& id, bool exact = true);

  bool
  setEntry_Run(RunID const& id, bool exact = true);

  void
  rewind();

  void
  setToLastEntry();

  void
  nextEntry();

  void
  previousEntry();

  void
  advanceEntry(std::size_t n);

  unsigned
  eventsToSkip() const;

  int
  skipEvents(int offset);

  int
  setForcedRunOffset(RunNumber_t const& forcedRunNumber);

  //bool
  //nextEventEntry();

  FileIndex::EntryType
  getEntryType() const;

  FileIndex::EntryType
  getNextEntryTypeWanted();

  std::shared_ptr<FileIndex>
  fileIndexSharedPtr() const;

  EventID
  eventIDForFileIndexPosition() const;

  std::vector<std::string> const&
  secondaryFileNames() const;

  std::vector<std::unique_ptr<RootInputFile>> const&
  secondaryFiles() const;

  void
  openSecondaryFile(int const idx);

  std::unique_ptr<RangeSetHandler>
  runRangeSetHandler();

  std::unique_ptr<RangeSetHandler>
  subRunRangeSetHandler();

private: // MEMBER FUNCTIONS -- Implementation details

  RootInputTree const&
  eventTree() const;

  RootInputTree const&
  subRunTree() const;

  RootInputTree const&
  runTree() const;

  RootInputTree const&
  resultsTree() const;

  RootInputTree&
  eventTree();

  RootInputTree&
  subRunTree();

  RootInputTree&
  runTree();

  RootInputTree&
  resultsTree();

  bool
  setIfFastClonable(FastCloningInfoProvider const& fcip) const;

  void
  validateFile();

  void
  fillHistory(EntryNumber const entry, History&);

  std::array<AvailableProducts_t, NumBranchTypes>
  fillPerBranchTypePresenceFlags(ProductList const&);

  void
  fillAuxiliary_Event(EntryNumber const entry);

  void
  fillAuxiliary_SubRun(EntryNumber const entry);

  void
  fillAuxiliary_Run(EntryNumber const entry);

  void
  fillAuxiliary_Results(EntryNumber const entry);

  std::unique_ptr<RangeSetHandler>
  fillAuxiliary_SubRun(EntryNumbers const& entries);

  std::unique_ptr<RangeSetHandler>
  fillAuxiliary_Run(EntryNumbers const& entries);

  void
  overrideRunNumber(RunAuxiliary&);

  void
  overrideRunNumber(SubRunID& id);

  void
  overrideRunNumber(EventID& id, bool isRealData);

  void
  dropOnInput(GroupSelectorRules const& rules, bool dropDescendants, ProductList& branchDescriptions);

  void
  readParentageTree(unsigned int treeCacheSize);

  void
  readEventHistoryTree(unsigned int treeCacheSize);

  void
  initializeDuplicateChecker();

  std::pair<EntryNumbers, bool>
  getEntryNumbers(BranchType);

  std::unique_ptr<RunPrincipal>
  readCurrentRun(EntryNumbers const&);

  std::unique_ptr<SubRunPrincipal>
  readCurrentSubRun(EntryNumbers const&, cet::exempt_ptr<RunPrincipal const>);

  std::unique_ptr<EventPrincipal>
  readCurrentEvent(std::pair<EntryNumbers, bool> const&);

private: // MEMBER DATA

  std::string const
  fileName_;

  std::string const
  catalog_;

  ProcessConfiguration const&
  processConfiguration_;

  std::string const
  logicalFileName_;

  std::unique_ptr<TFile>
  filePtr_;

  // Start with invalid connection.
  cet::sqlite::Connection
  sqliteDB_{};

  EventID
  origEventID_;

  EventNumber_t
  eventsToSkip_;

  RootInputTreePtrArray
  treePointers_;

  bool
  delayedReadEventProducts_;

  bool
  delayedReadSubRunProducts_;

  bool
  delayedReadRunProducts_;

  InputSource::ProcessingMode
  processingMode_;

  int
  forcedRunOffset_;

  bool
  noEventSort_;

  std::shared_ptr<DuplicateChecker>
  duplicateChecker_;

  cet::exempt_ptr<RootInputFile>
  primaryFile_;

  std::vector<std::string>
  secondaryFileNames_;

  cet::exempt_ptr<RootInputFileSequence>
  rifSequence_;

  FileFormatVersion
  fileFormatVersion_{};

  std::shared_ptr<FileIndex>
  fileIndexSharedPtr_{new FileIndex};

  FileIndex&
  fileIndex_{*fileIndexSharedPtr_};

  FileIndex::const_iterator
  fiBegin_{fileIndex_.begin()};

  FileIndex::const_iterator
  fiEnd_{fileIndex_.end()};

  FileIndex::const_iterator
  fiIter_{fiBegin_};

  bool
  fastClonable_{false};

  EventAuxiliary
  eventAux_{};

  SubRunAuxiliary
  subRunAux_{};

  RunAuxiliary
  runAux_{};

  ResultsAuxiliary
  resultsAux_{};

  ProductTables
  presentProducts_{ProductTables::invalid()};

  std::unique_ptr<BranchIDLists>
  branchIDLists_{};

  TTree*
  eventHistoryTree_{nullptr};

  std::vector<std::unique_ptr<RootInputFile>>
  secondaryFiles_{};

  // We need to add the secondary principals to the primary
  // principal when they are delay read, so we need to keep
  // around a pointer to the primary.  Note that these are
  // always used in a situation where we are guaranteed that
  // primary exists.

  cet::exempt_ptr<EventPrincipal>
  primaryEP_{};

  cet::exempt_ptr<RunPrincipal>
  primaryRP_{};

  cet::exempt_ptr<SubRunPrincipal>
  primarySRP_{};

  std::unique_ptr<RangeSetHandler>
  subRunRangeSetHandler_{nullptr};

  std::unique_ptr<RangeSetHandler>
  runRangeSetHandler_{nullptr};

  int64_t
  saveMemoryObjectThreshold_;

};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootInputFile_h */
