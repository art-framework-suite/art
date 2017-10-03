#ifndef art_Framework_IO_Root_RootInputFile_h
#define art_Framework_IO_Root_RootInputFile_h
// vim: set sw=2:

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/RootInputFileSequence.h"
#include "art/Framework/IO/Root/RootInputTree.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
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
#include "cetlib/exempt_ptr.h"
#include "cetlib/sqlite/Connection.h"

#include <array>
#include <map>
#include <memory>
#include <string>
#include <vector>

class TFile;

namespace art {

  class DuplicateChecker;
  class EventRangeHandler;
  class GroupSelectorRules;
  class MasterProductRegistry;

  class RootInputFile {

  public: // TYPES

    using RootInputTreePtrArray = std::array<std::unique_ptr<RootInputTree>, NumBranchTypes>;
    using EntryNumber = RootInputTree::EntryNumber;
    using EntryNumbers = RootInputTree::EntryNumbers;

  public: // MEMBER FUNCTIONS

    RootInputFile(RootInputFile const&) = delete;

    RootInputFile&
    operator=(RootInputFile const&) = delete;

    RootInputFile(std::string const& fileName,
                  std::string const& catalogName,
                  ProcessConfiguration const& processConfiguration,
                  std::string const& logicalFileName,
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
                  std::shared_ptr<DuplicateChecker> duplicateChecker,
                  bool dropDescendantsOfDroppedProducts,
                  bool readIncomingParameterSets,
                  cet::exempt_ptr<RootInputFile> primaryFile,
                  std::vector<std::string> const& secondaryFileNames,
                  RootInputFileSequence* rifSequence,
                  MasterProductRegistry& mpr);

    void reportOpened();

    void close(bool reallyClose);

    std::unique_ptr<ResultsPrincipal> readResults();
    std::unique_ptr<RunPrincipal> readRun();
    std::unique_ptr<SubRunPrincipal> readSubRun(cet::exempt_ptr<RunPrincipal>);
    std::unique_ptr<EventPrincipal> readEvent();

    bool readRunForSecondaryFile(RunID);
    bool readSubRunForSecondaryFile(SubRunID);
    bool readEventForSecondaryFile(EventID eID);

    std::string const& fileName() const { return fileName_; }

    RunAuxiliary& runAux() { return std::get<RunAuxiliary>(auxiliaries_); }
    ResultsAuxiliary& resultsAux() { return std::get<ResultsAuxiliary>(auxiliaries_); }
    SubRunAuxiliary& subRunAux() { return std::get<SubRunAuxiliary>(auxiliaries_); }
    EventAuxiliary& eventAux() { return std::get<EventAuxiliary>(auxiliaries_); }

    FileFormatVersion fileFormatVersion() const { return fileFormatVersion_; }

    bool fastClonable() const { return fastClonable_; }

    std::unique_ptr<FileBlock> createFileBlock();

    template <BranchType BT>
    void setEntry(FileIndex::EntryNumber_t entry)
    {
      treePointers_[BT]->setEntryNumber(entry);
    }

    template <BranchType BT, typename ID>
    bool setEntry(ID const& id, bool exact = true)
    {
      fiIter_ = fileIndex_.findPosition(id, exact);
      if (fiIter_ == fiEnd_) {
        return false;
      }
      setEntry<BT>(fiIter_->entry_);
      return true;
    }

    void rewind()
    {
      fiIter_ = fiBegin_;
      // FIXME: Rewinding the trees is suspicious!
      // FIXME: They should be positioned based on the new iter pos.
      eventTree().rewind();
      subRunTree().rewind();
      runTree().rewind();
    }

    void setToLastEntry() { fiIter_ = fiEnd_; }
    void nextEntry() { ++fiIter_; }
    void previousEntry() { --fiIter_; }
    void advanceEntry(std::size_t n) { while (n-- != 0) nextEntry(); }

    unsigned int eventsToSkip() const { return eventsToSkip_; }
    int skipEvents(int offset);
    int setForcedRunOffset(RunNumber_t const& forcedRunNumber);

    bool nextEventEntry() { return eventTree().next(); }

    FileIndex::EntryType getEntryType() const;
    FileIndex::EntryType getNextEntryTypeWanted();

    std::shared_ptr<FileIndex> fileIndexSharedPtr() const
    {
      return fileIndexSharedPtr_;
    }

    EventID eventIDForFileIndexPosition() const;

    auto const& secondaryFileNames() const { return secondaryFileNames_; }
    auto const& secondaryFiles() const { return secondaryFiles_; }

    void openSecondaryFile(int const idx);

    std::unique_ptr<RangeSetHandler> runRangeSetHandler();
    std::unique_ptr<RangeSetHandler> subRunRangeSetHandler();

  private:

    // const versions
    RootInputTree const& eventTree() const { return *treePointers_[InEvent]; }
    RootInputTree const& subRunTree() const { return *treePointers_[InSubRun]; }
    RootInputTree const& runTree() const { return *treePointers_[InRun]; }
    RootInputTree const& resultsTree() const { return *treePointers_[InResults]; }

    // non-const versions
    RootInputTree& eventTree() { return *treePointers_[InEvent]; }
    RootInputTree& subRunTree() { return *treePointers_[InSubRun]; }
    RootInputTree& runTree() { return *treePointers_[InRun]; }
    RootInputTree& resultsTree() { return *treePointers_[InResults]; }

    bool setIfFastClonable(FastCloningInfoProvider const& fcip) const;

    void validateFile();

    void fillHistory();
    std::array<AvailableProducts_t, NumBranchTypes> fillPerBranchTypePresenceFlags(ProductList const&);

    template <BranchType BT>
    void fillAuxiliary(EntryNumber const entry)
    {
      using AUX = std::tuple_element_t<BT,decltype(auxiliaries_)>;
      auto& aux = std::get<BT>(auxiliaries_);
      aux = treePointers_[BT]->getAux<AUX>(entry);
    }

    template <BranchType BT>
    std::unique_ptr<RangeSetHandler> fillAuxiliary(EntryNumbers const& entries)
    {
      using AUX = std::tuple_element_t<BT,decltype(auxiliaries_)>;
      auto& aux = std::get<BT>(auxiliaries_);
      return treePointers_[BT]->fillAux<AUX>(fileFormatVersion_,
                                             entries,
                                             sqliteDB_,
                                             fileName_,
                                             aux);
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

    std::pair<EntryNumbers,bool> getEntryNumbers(BranchType);

    std::unique_ptr<RunPrincipal> readCurrentRun(EntryNumbers const&);
    std::unique_ptr<SubRunPrincipal> readCurrentSubRun(EntryNumbers const&,
                                                       cet::exempt_ptr<RunPrincipal>);
    std::unique_ptr<EventPrincipal> readCurrentEvent(std::pair<EntryNumbers,bool> const&);

    std::string const fileName_;
    std::string const catalog_;
    ProcessConfiguration const& processConfiguration_;
    std::string const logicalFileName_;
    std::unique_ptr<TFile> filePtr_;
    cet::sqlite::Connection sqliteDB_{}; // Start with invalid connection.
    EventID origEventID_;
    EventNumber_t eventsToSkip_;
    bool const compactSubRunRanges_;
    RootInputTreePtrArray treePointers_;
    bool delayedReadEventProducts_;
    bool delayedReadSubRunProducts_;
    bool delayedReadRunProducts_;
    InputSource::ProcessingMode processingMode_;
    int forcedRunOffset_;
    bool noEventSort_;
    std::shared_ptr<DuplicateChecker> duplicateChecker_;
    cet::exempt_ptr<RootInputFile> primaryFile_;
    std::vector<std::string> secondaryFileNames_;
    cet::exempt_ptr<RootInputFileSequence> rifSequence_;

    FileFormatVersion fileFormatVersion_ {};
    std::shared_ptr<FileIndex> fileIndexSharedPtr_ { new FileIndex };
    FileIndex& fileIndex_ { *fileIndexSharedPtr_ };
    FileIndex::const_iterator fiBegin_ {fileIndex_.begin()};
    FileIndex::const_iterator fiEnd_ {fileIndex_.end()};
    FileIndex::const_iterator fiIter_ {fiBegin_};
    bool fastClonable_ {false};
    std::tuple<EventAuxiliary,
               SubRunAuxiliary,
               RunAuxiliary,
               ResultsAuxiliary> auxiliaries_{};   // Must be in same order as treePointers_ !

    // The holder is necessary since references of its contents are
    // passed to the RootDelayedReader.
    ProductRegistry productListHolder_{};
    ProductTables presentProducts_{ProductTables::invalid()};
    std::unique_ptr<BranchIDLists> branchIDLists_{nullptr}; // Only used for maintaining backwards compatibility

    TTree* eventHistoryTree_{nullptr};
    std::shared_ptr<History> history_{std::make_shared<History>()};
    std::vector<std::unique_ptr<RootInputFile>> secondaryFiles_{};
    // We need to add the secondary principals to the primary
    // principal when they are delay read, so we need to keep around a
    // pointer to the primary.  Note that these are always used in a
    // situation where we are guaranteed that primary exists.
    cet::exempt_ptr<EventPrincipal> primaryEP_{};
    cet::exempt_ptr<RunPrincipal> primaryRP_{};
    cet::exempt_ptr<SubRunPrincipal> primarySRP_{};
    std::unique_ptr<RangeSetHandler> subRunRangeSetHandler_{nullptr};
    std::unique_ptr<RangeSetHandler> runRangeSetHandler_{nullptr};

  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_IO_Root_RootInputFile_h */
