#ifndef art_Framework_IO_Root_RootInputFile_h
#define art_Framework_IO_Root_RootInputFile_h
// vim: set sw=2:

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/RootInputFileSequence.h"
#include "art/Framework/IO/Root/RootTree.h"
#include "art/Persistency/Provenance/BranchChildren.h"
#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Persistency/Provenance/History.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/array"
#include "cpp0x/memory"
#include <map>
#include <string>
#include <vector>

class TFile;

namespace art {

class DuplicateChecker;
class GroupSelectorRules;

class RootInputFile {

public: // TYPES

  typedef std::array<RootTree*, NumBranchTypes> RootTreePtrArray;

public: // MEMBER FUNCTIONS

  RootInputFile(RootInputFile const&) = delete;

  RootInputFile&
  operator=(RootInputFile const&) = delete;

  RootInputFile(std::string const& fileName,
                std::string const& catalogName,
                ProcessConfiguration const& processConfiguration,
                std::string const& logicalFileName,
                std::shared_ptr<TFile> filePtr,
                EventID const& origEventID,
                unsigned int eventsToSkip,
                std::vector<SubRunID> const& whichSubRunsToSkip,
                FastCloningInfoProvider const& fcip,
                unsigned int treeCacheSize,
                int64_t treeMaxVirtualSize,
                int64_t saveMemoryObjectThreashold,
                bool delayedReadSubRunProducts,
                bool delayedReadRunProducts,
                InputSource::ProcessingMode processingMode,
                int forcedRunOffset,
                bool noEventSort,
                GroupSelectorRules const& groupSelectorRules,
                bool dropMergeable,
                std::shared_ptr<DuplicateChecker> duplicateChecker,
                bool dropDescendantsOfDroppedProducts,
                cet::exempt_ptr<RootInputFile> primaryFile,
                int secondaryFileNameIdx,
                std::vector<std::string> const& secondaryFileNames,
                RootInputFileSequence* rifSequence);

  void
  reportOpened();

  void
  close(bool reallyClose);

  std::unique_ptr<EventPrincipal>
  readEvent();

  std::unique_ptr<EventPrincipal>
  readCurrentEvent();

  bool
  readEventForSecondaryFile(EventID eID);

  std::shared_ptr<RunPrincipal>
  readRun();

  std::vector<std::shared_ptr<RunPrincipal>>
  readRunFromSecondaryFiles();

  bool
  readRunForSecondaryFile(RunID);

  std::shared_ptr<SubRunPrincipal>
  readSubRun(std::shared_ptr<RunPrincipal>);

  std::vector<std::shared_ptr<SubRunPrincipal>>
  readSubRunFromSecondaryFiles(std::shared_ptr<RunPrincipal>);

  bool
  readSubRunForSecondaryFile(SubRunID);

  std::string const&
  file() const
  {
    return file_;
  }

  ProductList const&
  productList() const
  {
    return productListHolder_->productList_;
  }

  BranchIDListRegistry::collection_type const&
  branchIDLists()
  {
    return *branchIDLists_;
  }

  EventAuxiliary const&
  eventAux() const
  {
    return eventAux_;
  }

  SubRunAuxiliary const&
  subRunAux()
  {
    return subRunAux_;
  }

  RunAuxiliary const&
  runAux() const
  {
    return runAux_;
  }

  EventID const&
  eventID() const
  {
    return eventAux().id();
  }

  RootTreePtrArray&
  treePointers()
  {
    return treePointers_;
  }

  RootTree const&
  eventTree() const
  {
    return eventTree_;
  }

  RootTree const&
  subRunTree() const
  {
    return subRunTree_;
  }

  RootTree const&
  runTree() const
  {
    return runTree_;
  }

  FileFormatVersion
  fileFormatVersion() const
  {
    return fileFormatVersion_;
  }

  bool
  fastClonable() const
  {
    return fastClonable_;
  }

  std::shared_ptr<FileBlock>
  createFileBlock() const;

  bool
  setEntryAtEvent(EventID const& eID, bool exact);

  bool
  setEntryAtSubRun(SubRunID const& subRun);

  bool
  setEntryAtRun(RunID const& run);

  void
  setAtEventEntry(FileIndex::EntryNumber_t entry);

  void
  setAtRunEntry(FileIndex::EntryNumber_t entry);

  void
  setAtSubRunEntry(FileIndex::EntryNumber_t entry);

  void
  rewind()
  {
    fiIter_ = fiBegin_;
    // FIXME: Rewinding the trees is suspicious!
    // FIXME: They should be positioned based on the new iter pos.
    eventTree_.rewind();
    subRunTree_.rewind();
    runTree_.rewind();
    //updateSecondaryIter();
  }

  void
  setToLastEntry()
  {
    fiIter_ = fiEnd_;
    //updateSecondaryIter();
  }

  void
  nextEntry()
  {
    ++fiIter_;
    //updateSecondaryIter();
  }

  void
  previousEntry()
  {
    --fiIter_;
    //updateSecondaryIter();
  }

  using PerBranchTypePresence = MasterProductRegistry::PerBranchTypePresence;

  PerBranchTypePresence
  perBranchTypePresence()
  {
    return perBranchTypeProdPresence_;
  }

  unsigned int
  eventsToSkip() const
  {
    return eventsToSkip_;
  }

  int
  skipEvents(int offset);

  int
  setForcedRunOffset(RunNumber_t const& forcedRunNumber);

  bool
  nextEventEntry()
  {
    return eventTree_.next();
  }

  FileIndex::EntryType
  getEntryType() const;

  FileIndex::EntryType
  getEntryTypeSkippingDups();

  FileIndex::EntryType
  getNextEntryTypeWanted();

  std::shared_ptr<FileIndex>
  fileIndexSharedPtr() const
  {
    return fileIndexSharedPtr_;
  }

  EventID
  eventIDForFileIndexPosition() const;

  std::vector<std::string> const&
  secondaryFileNames() const
  {
    return secondaryFileNames_;
  }

  std::vector<std::unique_ptr<RootInputFile> > const &
  secondaryFiles() const
  {
    return secondaryFiles_;
  }

  void
  openSecondaryFile(int const idx);

private:

  bool setIfFastClonable(FastCloningInfoProvider const& fcip) const;

  void validateFile();

  void fillHistory();
  void fillPerBranchTypePresenceFlags(ProductList const&);
  void fillEventAuxiliary();
  void fillSubRunAuxiliary();
  void fillRunAuxiliary();

  void overrideRunNumber(RunID& id);
  void overrideRunNumber(SubRunID& id);
  void overrideRunNumber(EventID& id, bool isRealData);

  void dropOnInput(GroupSelectorRules const& rules,
                   bool dropDescendants,
                   bool dropMergeable,
                   ProductList& branchDescriptions);

  void readParentageTree();
  void readEventHistoryTree();

  void initializeDuplicateChecker();

  std::unique_ptr<RunPrincipal   > readCurrentRun();
  std::unique_ptr<SubRunPrincipal> readCurrentSubRun(std::shared_ptr<RunPrincipal>);

  std::string const file_;
  std::string const logicalFile_;
  std::string const catalog_;
  bool delayedReadSubRunProducts_;
  bool delayedReadRunProducts_;
  ProcessConfiguration const& processConfiguration_;
  std::shared_ptr<TFile> filePtr_;
  FileFormatVersion fileFormatVersion_;
  std::shared_ptr<FileIndex> fileIndexSharedPtr_;
  FileIndex& fileIndex_;
  FileIndex::const_iterator fiBegin_;
  FileIndex::const_iterator fiEnd_;
  FileIndex::const_iterator fiIter_;
  EventID origEventID_;
  EventNumber_t eventsToSkip_;
  std::vector<SubRunID> whichSubRunsToSkip_;
  bool noEventSort_;
  bool fastClonable_;
  EventAuxiliary eventAux_;
  SubRunAuxiliary subRunAux_;
  RunAuxiliary runAux_;
  RootTree eventTree_;
  RootTree subRunTree_;
  RootTree runTree_;
  RootTreePtrArray treePointers_;
  std::unique_ptr<ProductRegistry> productListHolder_;
  std::shared_ptr<BranchIDListRegistry::collection_type const> branchIDLists_;

  PerBranchTypePresence perBranchTypeProdPresence_;
  InputSource::ProcessingMode processingMode_;
  int forcedRunOffset_;
  TTree* eventHistoryTree_;
  std::shared_ptr<History> history_;
  std::shared_ptr<BranchChildren> branchChildren_;
  std::shared_ptr<DuplicateChecker> duplicateChecker_;
  cet::exempt_ptr<RootInputFile> primaryFile_;
  int secondaryFileNameIdx_;
  std::vector<std::string> secondaryFileNames_;
  std::vector<std::unique_ptr<RootInputFile> > secondaryFiles_;
  cet::exempt_ptr<RootInputFileSequence> rifSequence_;
  // We need to add the secondary principals to the primary
  // principal when they are delay read, so we need to keep
  // around a pointer to the primary.  Note that these are
  // always used in a situation where we are guaranteed that
  // primary exists.
  cet::exempt_ptr<EventPrincipal> primaryEP_;
  cet::exempt_ptr<RunPrincipal> primaryRP_;
  cet::exempt_ptr<SubRunPrincipal> primarySRP_;
  // The event processor reads run and subRun principals through
  // and interface that can return only the primary one.  These
  // data members cache the secondary ones so that the event
  // processor can collect them with a second call.  The secondary
  // event principals do not need to be collected since they are
  // never subjected to merging of their data products.
  std::vector<std::shared_ptr<Principal>> secondaryRPs_;
  std::vector<std::shared_ptr<Principal>> secondarySRPs_;

};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif // art_Framework_IO_Root_RootInputFile_h
