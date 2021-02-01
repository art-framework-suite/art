#ifndef art_Framework_IO_Root_RootInputFile_h
#define art_Framework_IO_Root_RootInputFile_h
// vim: set sw=2:

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/RootInputTree.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
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
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/sqlite/Connection.h"

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "TFile.h"

namespace art {

  class DuplicateChecker;
  class EventRangeHandler;
  class GroupSelectorRules;
  class MasterProductRegistry;

  class RootInputFile {

  public: // TYPES
    using RootInputTreePtrArray =
      std::array<std::unique_ptr<RootInputTree>, NumBranchTypes>;
    using EntryNumber = RootInputTree::EntryNumber;
    using EntryNumbers = RootInputTree::EntryNumbers;

  public: // MEMBER FUNCTIONS
    RootInputFile(RootInputFile const&) = delete;

    RootInputFile& operator=(RootInputFile const&) = delete;

    RootInputFile(std::string const& fileName,
                  ProcessConfiguration const& processConfiguration,
                  std::unique_ptr<TFile>&& filePtr,
                  EventID const& origEventID,
                  unsigned int eventsToSkip,
                  bool compactSubRunRanges,
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
                  bool dropDescendantsOfDroppedProducts,
                  bool readIncomingParameterSets,
                  MasterProductRegistry& mpr,
                  std::shared_ptr<DuplicateChecker> duplicateChecker = nullptr,
                  secondary_opener_t secondaryFileOpener = {});

    void reportOpened();
    void close();

    // Assumes sequential access
    std::unique_ptr<ResultsPrincipal> readResults();
    std::unique_ptr<RunPrincipal> readRun();
    std::unique_ptr<SubRunPrincipal> readSubRun();
    std::unique_ptr<EventPrincipal> readEvent();

    // Random access
    std::unique_ptr<RunPrincipal> readRunWithID(
      RunID id,
      bool thenAdvanceToNextRun = false);
    std::unique_ptr<SubRunPrincipal> readSubRunWithID(
      SubRunID id,
      bool thenAdvanceToNextSubRun = false);
    std::unique_ptr<EventPrincipal> readEventWithID(EventID const& id);

    std::string const&
    fileName() const
    {
      return fileName_;
    }

    FileFormatVersion
    fileFormatVersion() const
    {
      return fileFormatVersion_;
    }

    std::unique_ptr<FileBlock> createFileBlock();

    void
    setEntry(BranchType const BT, FileIndex::EntryNumber_t const entry)
    {
      treePointers_[BT]->setEntryNumber(entry);
    }

    template <typename ID>
    bool
    setEntry(BranchType const BT, ID const& id, bool const exact = true)
    {
      fiIter_ = fileIndex_.findPosition(id, exact);
      if (fiIter_ == fiEnd_) {
        return false;
      }
      setEntry(BT, fiIter_->entry_);
      return true;
    }

    void
    rewind()
    {
      fiIter_ = fiBegin_;
      // FIXME: Rewinding the trees is suspicious!
      // FIXME: They should be positioned based on the new iter pos.
      eventTree().rewind();
      subRunTree().rewind();
      runTree().rewind();
    }

    void
    setToLastEntry()
    {
      fiIter_ = fiEnd_;
    }
    void
    nextEntry()
    {
      ++fiIter_;
    }
    void
    previousEntry()
    {
      --fiIter_;
    }
    void
    advanceEntry(std::size_t n)
    {
      while (n-- != 0)
        nextEntry();
    }

    unsigned int
    eventsToSkip() const
    {
      return eventsToSkip_;
    }
    int skipEvents(int offset);
    int setForcedRunOffset(RunNumber_t const& forcedRunNumber);

    bool
    nextEventEntry()
    {
      return eventTree().next();
    }

    FileIndex::EntryType getEntryType() const;
    FileIndex::EntryType getNextEntryTypeWanted();

    std::shared_ptr<FileIndex>
    fileIndexSharedPtr() const
    {
      return fileIndexSharedPtr_;
    }

    EventID eventIDForFileIndexPosition() const;

    std::unique_ptr<RangeSetHandler> runRangeSetHandler();
    std::unique_ptr<RangeSetHandler> subRunRangeSetHandler();

  private:
    // const versions
    RootInputTree const&
    eventTree() const
    {
      return *treePointers_[InEvent];
    }
    RootInputTree const&
    subRunTree() const
    {
      return *treePointers_[InSubRun];
    }
    RootInputTree const&
    runTree() const
    {
      return *treePointers_[InRun];
    }
    RootInputTree const&
    resultsTree() const
    {
      return *treePointers_[InResults];
    }

    // non-const versions
    RootInputTree&
    eventTree()
    {
      return *treePointers_[InEvent];
    }
    RootInputTree&
    subRunTree()
    {
      return *treePointers_[InSubRun];
    }
    RootInputTree&
    runTree()
    {
      return *treePointers_[InRun];
    }
    RootInputTree&
    resultsTree()
    {
      return *treePointers_[InResults];
    }

    bool setIfFastClonable(FastCloningInfoProvider const& fcip) const;

    void validateFile();

    void fillHistory();
    std::array<AvailableProducts_t, NumBranchTypes>
    fillPerBranchTypePresenceFlags(ProductList const&);

    template <typename T>
    std::pair<T, std::unique_ptr<RangeSetHandler>>
    fillAuxiliary(EntryNumbers const& entries)
    {
      T aux;
      auto rs = treePointers_[T::branch_type]->template fillAux<T>(
        fileFormatVersion_, entries, fileIndex_, sqliteDB_, fileName_, aux);
      return std::make_pair(aux, move(rs));
    }

    void overrideRunNumber(RunID& id);
    void overrideRunNumber(SubRunID& id);
    void overrideRunNumber(EventID& id, bool isRealData);

    void dropOnInput(GroupSelectorRules const& rules,
                     bool dropDescendants,
                     ProductList& branchDescriptions);

    void readParentageTree(unsigned int treeCacheSize);
    void readEventHistoryTree(unsigned int treeCacheSize);

    void initializeDuplicateChecker();

    std::pair<EntryNumbers, bool> getEntryNumbers(BranchType);

    std::string const fileName_;
    ProcessConfiguration const& processConfiguration_;
    std::unique_ptr<TFile> filePtr_;
    cet::sqlite::Connection sqliteDB_{}; // Start with invalid connection.
    EventID origEventID_;
    EventNumber_t eventsToSkip_;
    bool const compactSubRunRanges_;
    secondary_opener_t secondaryFileOpener_;
    RootInputTreePtrArray treePointers_;
    bool delayedReadEventProducts_;
    bool delayedReadSubRunProducts_;
    bool delayedReadRunProducts_;
    InputSource::ProcessingMode processingMode_;
    int forcedRunOffset_;
    bool noEventSort_;
    std::shared_ptr<DuplicateChecker> duplicateChecker_;

    FileFormatVersion fileFormatVersion_{};
    std::shared_ptr<FileIndex> fileIndexSharedPtr_{new FileIndex};
    FileIndex& fileIndex_{*fileIndexSharedPtr_};
    FileIndex::const_iterator fiBegin_{fileIndex_.begin()};
    FileIndex::const_iterator fiEnd_{fileIndex_.end()};
    FileIndex::const_iterator fiIter_{fiBegin_};
    bool fastClonable_{false};

    // The holder is necessary since references of its contents are
    // passed to the RootDelayedReader.
    ProductRegistry productListHolder_{};
    ProductTables presentProducts_{ProductTables::invalid()};
    std::unique_ptr<BranchIDLists> branchIDLists_{
      nullptr}; // Only used for maintaining backwards compatibility

    TTree* eventHistoryTree_{nullptr};
    std::shared_ptr<History> history_{std::make_shared<History>()};
    std::unique_ptr<RangeSetHandler> subRunRangeSetHandler_{nullptr};
    std::unique_ptr<RangeSetHandler> runRangeSetHandler_{nullptr};
  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootInputFile_h */
