#include "art/Framework/IO/Root/RootInputFile.h"
// vim: set sw=2:

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/GroupSelector.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/IO/Root/DuplicateChecker.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/detail/resolveRangeSet.h"
#include "art/Framework/IO/Root/detail/setFileIndexPointer.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/BranchChildren.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ParameterSetBlob.h"
#include "canvas/Persistency/Provenance/ParameterSetMap.h"
#include "canvas/Persistency/Provenance/ParentageRegistry.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/FriendlyName.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "Rtypes.h"
#include "TClass.h"
#include "TFile.h"
#include "TTree.h"

#include <algorithm>
#include <utility>

extern "C" {
#include "sqlite3.h"
}

#include <iostream>
#include <string>

using namespace cet;
using namespace std;

namespace {

  bool have_table(sqlite3 * db,
                  std::string const& table,
                  std::string const& filename)
  {
    bool result = false;
    sqlite3_stmt* stmt = nullptr;
    std::string const ddl {"select 1 from sqlite_master where type='table' and name='"+table+"';"};
    auto rc = sqlite3_prepare_v2(db, ddl.c_str(), -1, &stmt, nullptr);
    if (rc == SQLITE_OK) {
      switch (rc = sqlite3_step(stmt)) {
      case SQLITE_ROW:
        result = true; // Found the table.
      case SQLITE_DONE:
        rc = SQLITE_OK; // No such table.
        break;
      default:
        break;
      }
    }
    rc = sqlite3_finalize(stmt);
    if (rc != SQLITE_OK) {
      throw art::Exception(art::errors::FileReadError)
        << "Error interrogating SQLite3 DB in file " << filename
        << ".\n";
    }
    return result;
  }

}

namespace art {

  RootInputFile::
  RootInputFile(string const& fileName,
                string const& catalogName,
                ProcessConfiguration const& processConfiguration,
                string const& logicalFileName,
                shared_ptr<TFile> filePtr,
                EventID const& origEventID,
                unsigned int eventsToSkip,
                vector<SubRunID> const& whichSubRunsToSkip,
                FastCloningInfoProvider const& fcip,
                unsigned int treeCacheSize,
                int64_t treeMaxVirtualSize,
                int64_t saveMemoryObjectThreshold,
                bool delayedReadSubRunProducts,
                bool delayedReadRunProducts,
                InputSource::ProcessingMode processingMode,
                int forcedRunOffset,
                bool noEventSort,
                GroupSelectorRules const& groupSelectorRules,
                bool dropMergeable,
                shared_ptr<DuplicateChecker> duplicateChecker,
                bool dropDescendants,
                bool const readIncomingParameterSets,
                exempt_ptr<RootInputFile> primaryFile,
                int secondaryFileNameIdx,
                vector<string> const& secondaryFileNames,
                RootInputFileSequence* rifSequence)
    : file_{fileName}
    , catalog_{catalogName}
    , processConfiguration_{processConfiguration}
    , logicalFile_{logicalFileName}
    , filePtr_{filePtr}
    , origEventID_{origEventID}
    , eventsToSkip_{eventsToSkip}
    , whichSubRunsToSkip_{whichSubRunsToSkip}
    , treePointers_ { // Order (and number) must match BranchTypes.h!
      std::make_unique<RootTree>(filePtr_, InEvent, saveMemoryObjectThreshold, this),
      std::make_unique<RootTree>(filePtr_, InSubRun, saveMemoryObjectThreshold, this),
      std::make_unique<RootTree>(filePtr_, InRun, saveMemoryObjectThreshold, this),
      std::make_unique<RootTree>(filePtr_, InResults, saveMemoryObjectThreshold, this, true /* missingOK */) }
    , delayedReadSubRunProducts_{delayedReadSubRunProducts}
    , delayedReadRunProducts_{delayedReadRunProducts}
    , processingMode_{processingMode}
    , forcedRunOffset_{forcedRunOffset}
    , noEventSort_{noEventSort}
    , duplicateChecker_{duplicateChecker}
    , primaryFile_{primaryFile ? primaryFile : this}
    , secondaryFileNameIdx_{secondaryFileNameIdx}
    , secondaryFileNames_{secondaryFileNames}
    , rifSequence_{rifSequence}
  {
    secondaryFiles_.resize(secondaryFileNames_.size());
    eventTree().setCacheSize(treeCacheSize);
    eventTree().setTreeMaxVirtualSize(treeMaxVirtualSize);
    subRunTree().setTreeMaxVirtualSize(treeMaxVirtualSize);
    runTree().setTreeMaxVirtualSize(treeMaxVirtualSize);
    if (resultsTree()) {
      resultsTree().setTreeMaxVirtualSize(treeMaxVirtualSize);
    }
    // Read the metadata tree.
    auto metaDataTree = static_cast<TTree*>(filePtr_->Get(rootNames::metaDataTreeName().c_str()));
    if (!metaDataTree) {
      throw art::Exception{errors::FileReadError}
        << couldNotFindTree(rootNames::metaDataTreeName());
    }
    auto fftPtr = &fileFormatVersion_;
    using namespace art::rootNames;
    metaDataTree->SetBranchAddress(metaBranchRootName<FileFormatVersion>(), &fftPtr);

    auto findexPtr = &fileIndex_;
    detail::setFileIndexPointer(filePtr_.get(), metaDataTree, findexPtr);
    auto plhPtr = productListHolder_.get();
    assert(plhPtr != nullptr &&
           "INTERNAL ERROR: productListHolder_ not initialized prior to use!.");
    metaDataTree->SetBranchAddress(metaBranchRootName<ProductRegistry>(), &plhPtr);
    if (plhPtr != productListHolder_.get()) {
      // Should never happen, but just in case ROOT's behavior changes.
      throw Exception{errors::LogicError}
        << "ROOT has changed behavior and caused a memory leak while setting "
        << "a branch address.";
    }
    ParameterSetMap psetMap;
    auto psetMapPtr = &psetMap;
    if (readIncomingParameterSets &&
        metaDataTree->GetBranch(metaBranchRootName<ParameterSetMap>())) {
      // May be in MetaData tree or DB.
      metaDataTree->SetBranchAddress(metaBranchRootName<ParameterSetMap>(),
                                     &psetMapPtr);
    }
    ProcessHistoryMap pHistMap;
    auto pHistMapPtr = &pHistMap;
    metaDataTree->SetBranchAddress(metaBranchRootName<ProcessHistoryMap>(), &pHistMapPtr);

    auto branchIDListsAPtr = std::make_unique<BranchIDLists>();
    auto branchIDListsPtr = branchIDListsAPtr.get();
    metaDataTree->SetBranchAddress(metaBranchRootName<BranchIDLists>(), &branchIDListsPtr);

    auto branchChildrenBuffer = branchChildren_.get();
    metaDataTree->SetBranchAddress(metaBranchRootName<BranchChildren>(), &branchChildrenBuffer);

    // Here we read the metadata tree
    input::getEntry(metaDataTree, 0);
    branchIDLists_.reset(branchIDListsAPtr.release());
    // Check the, "Era" of the input file (new since art v0.5.0). If it
    // does not match what we expect we cannot read the file. Required
    // since we reset the file versioning since forking off from
    // CMS. Files written by art prior to v0.5.0 will *also* not be
    // readable because they do not have this datum and because the run,
    // subrun and event-number handling has changed significantly.
    string const& expected_era = art::getFileFormatEra();
    if (fileFormatVersion_.era_ != expected_era) {
      throw art::Exception{art::errors::FileReadError}
        << "Can only read files written during the \""
        << expected_era
        << "\" era: "
        << "Era of "
        << "\""
        << file_
        << "\" was "
        << (fileFormatVersion_.era_.empty() ?
            "not set" :
            ("set to \"" + fileFormatVersion_.era_ + "\" "))
        << ".\n";
    }
    // Merge into the hashed registries.
    // Parameter Set
    for (auto const& psEntry : psetMap) {
      fhicl::ParameterSet pset;
      fhicl::make_ParameterSet(psEntry.second.pset_, pset);
      // Note ParameterSet::id() has the side effect of
      // making sure the parameter set *has* an ID.
      pset.id();
      fhicl::ParameterSetRegistry::put(pset);
    }

    // Also need to check RootFileDB if we have one.
    if (fileFormatVersion_.value_ >= 5) {
      if ( readIncomingParameterSets &&
           have_table(sqliteDB_, "ParameterSets", file_)) {
        fhicl::ParameterSetRegistry::importFrom(sqliteDB_);
      }
      if ( art::ServiceRegistry::instance().isAvailable<art::FileCatalogMetadata>() &&
           have_table(sqliteDB_, "FileCatalog_metadata", file_)) {
        sqlite3_stmt* stmt {nullptr};
        sqlite3_prepare_v2(sqliteDB_,
                           "SELECT Name, Value from FileCatalog_metadata;",
                           -1,
                           &stmt,
                           nullptr);

        std::vector<std::pair<std::string,std::string>> md;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
          std::string const name {reinterpret_cast<char const*>(sqlite3_column_text(stmt, 0))};
          std::string const value {reinterpret_cast<char const*>(sqlite3_column_text(stmt, 1))};
          md.emplace_back(name, value);
        }
        int const finalize_status = sqlite3_finalize(stmt);
        if (finalize_status != SQLITE_OK) {
          throw art::Exception{art::errors::SQLExecutionError}
               << "Unexpected status from DB status cleanup: "
               << sqlite3_errmsg(sqliteDB_)
               << " (0x"
               << finalize_status
               <<").\n";
        }
        art::ServiceHandle<art::FileCatalogMetadata>{}->setMetadataFromInput(md);
      }
    }
    ProcessHistoryRegistry::put(pHistMap);
    validateFile();
    // Read the parentage tree.  Old format files are handled
    // internally in readParentageTree().
    readParentageTree();
    initializeDuplicateChecker();
    if (noEventSort_) {
      fileIndex_.sortBy_Run_SubRun_EventEntry();
    }
    fiIter_  = fileIndex_.begin();
    fiBegin_ = fileIndex_.begin();
    fiEnd_   = fileIndex_.end();
    readEventHistoryTree();

    auto& prodList = productListHolder_->productList_;

    fillPerBranchTypePresenceFlags(prodList);
    dropOnInput(groupSelectorRules, dropDescendants, /*unused*/dropMergeable, prodList);

    // Determine if this file is fast clonable.
    fastClonable_ = setIfFastClonable(fcip);
    reportOpened();
  }

  void
  RootInputFile::
  readParentageTree()
  {
    //
    //  Auxiliary routine for the constructor.
    //
    auto parentageTree = static_cast<TTree*>(filePtr_->Get(rootNames::parentageTreeName().c_str()));
    if (!parentageTree) {
      throw art::Exception{errors::FileReadError}
      << couldNotFindTree(rootNames::parentageTreeName());
    }
    ParentageID idBuffer;
    auto pidBuffer = &idBuffer;
    parentageTree->SetBranchAddress(rootNames::parentageIDBranchName().c_str(),
                                    &pidBuffer);
    Parentage parentageBuffer;
    auto pParentageBuffer = &parentageBuffer;
    parentageTree->SetBranchAddress(rootNames::parentageBranchName().c_str(),
                                    &pParentageBuffer);
    for (Long64_t i = 0, numEntries = parentageTree->GetEntries(); i < numEntries;
         ++i) {
      input::getEntry(parentageTree, i);
      if (idBuffer != parentageBuffer.id()) {
        throw art::Exception{errors::DataCorruption}
          << "Corruption of Parentage tree detected.\n";
      }
      ParentageRegistry::put(parentageBuffer);
    }
    parentageTree->SetBranchAddress(rootNames::parentageIDBranchName().c_str(), 0);
    parentageTree->SetBranchAddress(rootNames::parentageBranchName().c_str(), 0);
  }

  EventID
  RootInputFile::
  eventIDForFileIndexPosition() const
  {
    if (fiIter_ == fiEnd_) {
      return EventID{};
    }
    return fiIter_->eventID_;
  }


  bool
  RootInputFile::
  setIfFastClonable(FastCloningInfoProvider const& fcip) const
  {
    if (!fcip.fastCloningPermitted()) {
      return false;
    }
    if (!fileFormatVersion_.fastCopyPossible()) {
      return false;
    }
    if (secondaryFileNames_.size() != 0) {
      return false;
    }
    if (!fileIndex_.allEventsInEntryOrder()) {
      return false;
    }
    if (eventsToSkip_ != 0) {
      return false;
    }
    if ((fcip.remainingEvents() >= 0) &&
        (eventTree().entries() > fcip.remainingEvents())) {
      return false;
    }
    if ((fcip.remainingSubRuns() >= 0) &&
        (subRunTree().entries() > fcip.remainingSubRuns())) {
      return false;
    }
    if (processingMode_ != InputSource::RunsSubRunsAndEvents) {
      return false;
    }
    if (forcedRunOffset_ != 0) {
      return false;
    }
    // Find entry for first event in file.
    auto it = fiBegin_;
    while ((it != fiEnd_) && (it->getEntryType() != FileIndex::kEvent)) {
      ++it;
    }
    if (it == fiEnd_) {
      return false;
    }
    if (it->eventID_ < origEventID_) {
      return false;
    }
    for (auto const& subrun : whichSubRunsToSkip_) {
      if (fileIndex_.findPosition(subrun, true) != fiEnd_) {
        // We must skip a subRun in this file.  We will simply assume that
        // it may contain an event, in which case we cannot fast copy.
        return false;
      }
    }
    return true;
  }


  int
  RootInputFile::
  setForcedRunOffset(RunNumber_t const& forcedRunNumber)
  {
    if (fiBegin_ == fiEnd_) {
      return 0;
    }
    forcedRunOffset_ = 0;
    if (!RunID(forcedRunNumber).isValid()) {
      return 0;
    }
    forcedRunOffset_ = forcedRunNumber - fiBegin_->eventID_.run();
    if (forcedRunOffset_ != 0) {
      fastClonable_ = false;
    }
    return forcedRunOffset_;
  }

  shared_ptr<FileBlock>
  RootInputFile::
  createFileBlock()
  {
    return std::make_shared<FileBlock>(fileFormatVersion_,
                                       eventTree().tree(),
                                       fastClonable(),
                                       file_,
                                       branchChildren_,
                                       readResults());
  }

  FileIndex::EntryType
  RootInputFile::
  getEntryType() const
  {
    if (fiIter_ == fiEnd_) {
      return FileIndex::kEnd;
    }
    return fiIter_->getEntryType();
  }

  // Temporary KLUDGE until we can properly merge runs and subRuns across files
  // This KLUDGE skips duplicate run or subRun entries.
  FileIndex::EntryType
  RootInputFile::
  getEntryTypeSkippingDups()
  {
    while (1) {
      if (fiIter_ == fiEnd_) {
        return FileIndex::kEnd;
      }
      if (fiIter_->eventID_.isValid() ||  // will not skip duplicate events
          (fiIter_ == fiBegin_) || // guarding next condition
          ((fiIter_ - 1)->eventID_.subRun() != fiIter_->eventID_.subRun())) { // do not skip unique subruns
        return fiIter_->getEntryType();
      }
      nextEntry();
    }
  }

  FileIndex::EntryType
  RootInputFile::
  getNextEntryTypeWanted()
  {
    auto entryType = getEntryTypeSkippingDups();
    if (entryType == FileIndex::kEnd) {
      return FileIndex::kEnd;
    }
    RunID currentRun(fiIter_->eventID_.runID());
    if (!currentRun.isValid()) {
      return FileIndex::kEnd;
    }
    if (entryType == FileIndex::kRun) {
      // Skip any runs before the first run specified
      if (currentRun < origEventID_.runID()) {
        fiIter_ = fileIndex_.findPosition(origEventID_.runID(), false);
        return getNextEntryTypeWanted();
      }
      return FileIndex::kRun;
    }
    if (processingMode_ == InputSource::Runs) {
      fiIter_ = fileIndex_.findPosition(currentRun.isValid() ?
                                        currentRun.next() : currentRun, false);
      return getNextEntryTypeWanted();
    }
    SubRunID const& currentSubRun = fiIter_->eventID_.subRunID();
    if (entryType == FileIndex::kSubRun) {
      // Skip any subRuns before the first subRun specified
      if ((currentRun == origEventID_.runID()) &&
          (currentSubRun < origEventID_.subRunID())) {
        fiIter_ = fileIndex_.findSubRunOrRunPosition(origEventID_.subRunID());
        return getNextEntryTypeWanted();
      }
      // Skip the subRun if it is in whichSubRunsToSkip_.
      if (binary_search_all(whichSubRunsToSkip_, currentSubRun)) {
        fiIter_ = fileIndex_.findSubRunOrRunPosition(currentSubRun.next());
        return getNextEntryTypeWanted();
      }
      return FileIndex::kSubRun;
    }
    if (processingMode_ == InputSource::RunsAndSubRuns) {
      fiIter_ = fileIndex_.findSubRunOrRunPosition(currentSubRun.next());
      return getNextEntryTypeWanted();
    }
    assert(entryType == FileIndex::kEvent);
    // Skip any events before the first event specified
    if (fiIter_->eventID_ < origEventID_) {
      fiIter_ = fileIndex_.findPosition(origEventID_);
      return getNextEntryTypeWanted();
    }
    if (duplicateChecker_.get() &&
        duplicateChecker_->isDuplicateAndCheckActive(fiIter_->eventID_, file_)) {
      nextEntry();
      return getNextEntryTypeWanted();
    }
    if (eventsToSkip_ == 0) {
      return FileIndex::kEvent;
    }
    // We have specified a count of events to skip,
    // keep skipping events in this subRun block
    // until we reach the end of the subRun block or
    // the full count of the number of events to skip.
    while ((eventsToSkip_ != 0) && (fiIter_ != fiEnd_) &&
           (getEntryTypeSkippingDups() == FileIndex::kEvent)) {
      nextEntry();
      --eventsToSkip_;
      while ((eventsToSkip_ != 0) && (fiIter_ != fiEnd_) &&
             (fiIter_->getEntryType() == FileIndex::kEvent) &&
             duplicateChecker_.get() &&
             duplicateChecker_->isDuplicateAndCheckActive(fiIter_->eventID_, file_)) {
        nextEntry();
      }
    }
    return getNextEntryTypeWanted();
  }

  void
  RootInputFile::
  validateFile()
  {
    if (!fileFormatVersion_.isValid()) {
      fileFormatVersion_.value_ = 0;
    }
    if (!eventTree().isValid()) {
      throw art::Exception{errors::DataCorruption}
        << "'Events' tree is corrupted or not present\n"
        << "in the input file.\n";
    }
    if (fileIndex_.empty()) {
      throw art::Exception{art::errors::FileReadError}
        << "FileIndex information is missing for the input file.\n";
    }
  }

  void
  RootInputFile::
  reportOpened()
  {
  }

  void
  RootInputFile::
  close(bool reallyClose)
  {
    if (!reallyClose) {
      return;
    }
    filePtr_->Close();
    for (auto const & sf : secondaryFiles_) {
      if (!sf) {
        continue;
      }
      sf->filePtr_->Close();
    }
  }

  void
  RootInputFile::
  fillHistory()
  {
    // We could consider doing delayed reading, but because we have to
    // store this History object in a different tree than the event
    // data tree, this is too hard to do in this first version.
    auto pHistory = history_.get();
    auto eventHistoryBranch = eventHistoryTree_->GetBranch(rootNames::eventHistoryBranchName().c_str());
    if (!eventHistoryBranch) {
      throw art::Exception{errors::DataCorruption}
        << "Failed to find history branch in event history tree.\n";
    }
    eventHistoryBranch->SetAddress(&pHistory);
    input::getEntry(eventHistoryTree_, eventTree().entryNumber());
  }

  int
  RootInputFile::
  skipEvents(int offset)
  {
    while ((offset > 0) && (fiIter_ != fiEnd_)) {
      if (fiIter_->getEntryType() == FileIndex::kEvent) {
        --offset;
      }
      nextEntry();
    }
    while ((offset < 0) && (fiIter_ != fiBegin_)) {
      previousEntry();
      if (fiIter_->getEntryType() == FileIndex::kEvent) {
        ++offset;
      }
    }
    while ((fiIter_ != fiEnd_) &&
           (fiIter_->getEntryType() != FileIndex::kEvent)) {
      nextEntry();
    }
    return offset;
  }

  // readEvent() is responsible for creating, and setting up, the
  // EventPrincipal.
  //
  //   1. create an EventPrincipal with a unique EventID
  //   2. For each entry in the provenance, put in one Group,
  //      holding the Provenance for the corresponding EDProduct.
  //   3. set up the caches in the EventPrincipal to know about this
  //      Group.
  //
  // We do *not* create the EDProduct instance (the equivalent of
  // reading the branch containing this EDProduct). That will be done
  // by the Delayed Reader, when it is asked to do so.
  //
  unique_ptr<EventPrincipal>
  RootInputFile::
  readEvent()
  {
    assert(fiIter_ != fiEnd_);
    assert(fiIter_->getEntryType() == FileIndex::kEvent);
    assert(fiIter_->eventID_.runID().isValid());

    auto const& entryNumbers = getEntryNumbers(InEvent);
    if (!eventTree().current(entryNumbers.first)) {
      // The supplied entry numbers are not valid.
      return nullptr;
    }

    auto ep = readCurrentEvent(entryNumbers);
    assert(ep);
    assert(eventAux().run() == fiIter_->eventID_.run() + forcedRunOffset_);
    assert(eventAux().subRunID() == fiIter_->eventID_.subRunID());
    nextEntry();
    return ep;
  }

  // Reads event at the current entry in the tree.
  // Note: This function neither uses nor sets fiIter_.
  unique_ptr<EventPrincipal>
  RootInputFile::
  readCurrentEvent(std::pair<EntryNumbers,bool> const& entryNumbers)
  {
    assert(entryNumbers.first.size() == 1ull);
    fillAuxiliary<InEvent>(entryNumbers.first.front());
    assert(eventAux().id() == fiIter_->eventID_);
    fillHistory();
    overrideRunNumber(const_cast<EventID&>(eventAux().id()), eventAux().isRealData());
    auto ep = std::make_unique<EventPrincipal>(eventAux(),
                                               processConfiguration_,
                                               history_,
                                               eventTree().makeBranchMapper(),
                                               eventTree().makeDelayedReader(fileFormatVersion_,
                                                                             InEvent,
                                                                             entryNumbers.first,
                                                                             eventAux().id()),
                                               entryNumbers.second,
                                               0,
                                               nullptr);
    eventTree().fillGroups(*ep);
    primaryEP_ = make_exempt_ptr(ep.get());
    return ep;
  }

  bool
  RootInputFile::
  readEventForSecondaryFile(EventID eID)
  {
    // Used just after opening a new secondary file in response to a failed
    // product lookup.  Synchronize the file index to the event needed and
    // create a secondary EventPrincipal for it.
    if (!setEntry<InEvent>(eID, /*exact=*/true)) {
      // Error, could not find specified event in file.
      return false;
    }
    auto const& entryNumbers = getEntryNumbers(InEvent);
    assert(entryNumbers.first.size() == 1ull);
    fillAuxiliary<InEvent>(entryNumbers.first.front());
    fillHistory();
    overrideRunNumber(const_cast<EventID&>(eventAux().id()),
                      eventAux().isRealData());
    auto sep = std::make_unique<EventPrincipal>(eventAux(),
                                                processConfiguration_,
                                                history_,
                                                eventTree().makeBranchMapper(),
                                                eventTree().makeDelayedReader(fileFormatVersion_,
                                                                              InEvent,
                                                                              entryNumbers.first,
                                                                              eventAux().id()),
                                                entryNumbers.second,
                                                secondaryFileNameIdx_ + 1,
                                                primaryFile_->primaryEP_.get());
    eventTree().fillGroups(*sep);
    primaryFile_->primaryEP_->addSecondaryPrincipal(unique_ptr<Principal>{move(sep)});
    return true;
  }

  unique_ptr<RangeSetHandler>
  RootInputFile::runRangeSetHandler()
  {
    return unique_ptr<ClosedRangeSetHandler>(runRangeSetHandler_.release());
  }

  shared_ptr<RunPrincipal>
  RootInputFile::
  readRun()
  {
    assert(fiIter_ != fiEnd_);
    assert(fiIter_->getEntryType() == FileIndex::kRun);
    assert(fiIter_->eventID_.runID().isValid());
    secondaryRPs_.clear();

    auto const& entryNumbers = getEntryNumbers(InRun).first;
    if (!runTree().current(entryNumbers)) {
      // The supplied entry numbers are not valid.
      return nullptr;
    }

    auto rp = readCurrentRun(entryNumbers);
    nextEntry();
    return move(rp);
  }

  vector<shared_ptr<RunPrincipal>>
  RootInputFile::
  readRunFromSecondaryFiles()
  {
    vector<shared_ptr<RunPrincipal>> rps;
    for (auto const& val : secondaryRPs_) {
      rps.emplace_back(dynamic_pointer_cast<RunPrincipal>(val));
    }
    return rps;
  }

  unique_ptr<RunPrincipal>
  RootInputFile::
  readCurrentRun(EntryNumbers const& entryNumbers)
  {
    runRangeSetHandler_ = fillAuxiliary<InRun>(entryNumbers);
    assert(runAux().id() == fiIter_->eventID_.runID());
    overrideRunNumber(runAux().id_);
    if (runAux().beginTime() == Timestamp::invalidTimestamp()) {
      // RunAuxiliary did not contain a valid timestamp.
      // Take it from the next event.
      if (eventTree().next()) {
        fillAuxiliary<InEvent>(eventTree().entryNumber());
        // back up, so event will not be skipped.
        eventTree().previous();
      }
      runAux().beginTime_ = eventAux().time();
      runAux().endTime_ = Timestamp::invalidTimestamp();
    }
    auto rp = std::make_unique<RunPrincipal>(runAux(),
                                             processConfiguration_,
                                             runTree().makeBranchMapper(),
                                             runTree().makeDelayedReader(fileFormatVersion_,
                                                                         sqliteDB_,
                                                                         InRun,
                                                                         entryNumbers,
                                                                         fiIter_->eventID_),
                                             0,
                                             nullptr);

    runTree().fillGroups(*rp);
    if (!delayedReadRunProducts_) {
      rp->readImmediate();
    }
    primaryRP_ = make_exempt_ptr(rp.get());
    return rp;
  }

  bool
  RootInputFile::
  readRunForSecondaryFile(RunID rID)
  {
    // Used just after opening a new secondary file in response to a failed
    // product lookup.  Synchronize the file index to the run needed and
    // create a secondary RunPrincipal for it.
    if (!setEntry<InRun>(rID)) {
      // Error, could not find specified run in file.
      return false;
    }
    auto const& entryNumbers = getEntryNumbers(InRun).first;
    assert(fiIter_ != fiEnd_);
    assert(fiIter_->getEntryType() == FileIndex::kRun);
    assert(fiIter_->eventID_.runID().isValid());
    runRangeSetHandler_ = fillAuxiliary<InRun>(entryNumbers);
    assert(runAux().id() == fiIter_->eventID_.runID());
    overrideRunNumber(runAux().id_);
    if (runAux().beginTime() == Timestamp::invalidTimestamp()) {
      // RunAuxiliary did not contain a valid timestamp.
      // Take it from the next event.
      if (eventTree().next()) {
        fillAuxiliary<InEvent>(eventTree().entryNumber());
        // back up, so event will not be skipped.
        eventTree().previous();
      }
      runAux().beginTime_ = eventAux().time();
      runAux().endTime_ = Timestamp::invalidTimestamp();
    }
    auto rp = std::make_shared<RunPrincipal>(runAux(),
                                             processConfiguration_,
                                             runTree().makeBranchMapper(),
                                             runTree().makeDelayedReader(fileFormatVersion_,
                                                                         sqliteDB_,
                                                                         InRun,
                                                                         entryNumbers,
                                                                         fiIter_->eventID_),
                                             secondaryFileNameIdx_ + 1,
                                             primaryFile_->primaryRP_.get());

    runTree().fillGroups(*rp);
    if (!delayedReadRunProducts_) {
      rp->readImmediate();
    }
    primaryFile_->primaryRP_->addSecondaryPrincipal(static_pointer_cast<Principal>(rp));
    // FIXME: These secondary run principals will never be cached
    // FIXME: by the event processor!  This means that we cannot
    // FIXME: output run data products from them.
    primaryFile_->secondaryRPs_.emplace_back(move(rp));
    return true;
  }

  unique_ptr<RangeSetHandler>
  RootInputFile::subRunRangeSetHandler()
  {
    return unique_ptr<ClosedRangeSetHandler>(subRunRangeSetHandler_.release());
  }

  shared_ptr<SubRunPrincipal>
  RootInputFile::
  readSubRun(shared_ptr<RunPrincipal> rp)
  {
    assert(fiIter_ != fiEnd_);
    assert(fiIter_->getEntryType() == FileIndex::kSubRun);
    secondarySRPs_.clear();

    auto const& entryNumbers = getEntryNumbers(InSubRun).first;
    if (!subRunTree().current(entryNumbers)) {
      // The supplied entry numbers are not valid.
      return nullptr;
    }

    auto srp = readCurrentSubRun(entryNumbers, rp);
    nextEntry();
    return move(srp);
  }

  vector<shared_ptr<SubRunPrincipal>>
  RootInputFile::
  readSubRunFromSecondaryFiles(shared_ptr<RunPrincipal>)
  {
    vector<shared_ptr<SubRunPrincipal>> srps;
    for (auto const& val : secondarySRPs_) {
      srps.emplace_back(dynamic_pointer_cast<SubRunPrincipal>(val));
    }
    return srps;
  }

  unique_ptr<SubRunPrincipal>
  RootInputFile::
  readCurrentSubRun(EntryNumbers const& entryNumbers,
                    shared_ptr<RunPrincipal> rp [[gnu::unused]])
  {
    subRunRangeSetHandler_ = fillAuxiliary<InSubRun>(entryNumbers);
    assert(subRunAux().id() == fiIter_->eventID_.subRunID());
    overrideRunNumber(subRunAux().id_);
    assert(subRunAux().runID() == rp->id());
    if (subRunAux().beginTime() == Timestamp::invalidTimestamp()) {
      // SubRunAuxiliary did not contain a timestamp.
      // Take it from the next event.
      if (eventTree().next()) {
        fillAuxiliary<InEvent>(eventTree().entryNumber());
        // back up, so event will not be skipped.
        eventTree().previous();
      }
      subRunAux().beginTime_ = eventAux().time();
      subRunAux().endTime_ = Timestamp::invalidTimestamp();
    }

    auto srp = std::make_unique<SubRunPrincipal>(subRunAux(),
                                                 processConfiguration_,
                                                 subRunTree().makeBranchMapper(),
                                                 subRunTree().makeDelayedReader(fileFormatVersion_,
                                                                                sqliteDB_,
                                                                                InSubRun,
                                                                                entryNumbers,
                                                                                fiIter_->eventID_),
                                                 0,
                                                 nullptr);

    subRunTree().fillGroups(*srp);
    if (!delayedReadSubRunProducts_) {
      srp->readImmediate();
    }
    primarySRP_ = make_exempt_ptr(srp.get());
    return srp;
  }

  bool
  RootInputFile::
  readSubRunForSecondaryFile(SubRunID srID)
  {
    // Used just after opening a new secondary file in response to a failed
    // product lookup.  Synchronize the file index to the subRun needed and
    // create a secondary SubRunPrincipal for it.
    if (!setEntry<InSubRun>(srID)) {
      // Error, could not find specified subRun in file.
      return false;
    }
    auto const& entryNumbers = getEntryNumbers(InSubRun).first;
    assert(fiIter_ != fiEnd_);
    assert(fiIter_->getEntryType() == FileIndex::kSubRun);
    subRunRangeSetHandler_ = fillAuxiliary<InSubRun>(entryNumbers);
    assert(subRunAux().id() == fiIter_->eventID_.subRunID());
    overrideRunNumber(subRunAux().id_);
    if (subRunAux().beginTime() == Timestamp::invalidTimestamp()) {
      // SubRunAuxiliary did not contain a timestamp.
      // Take it from the next event.
      if (eventTree().next()) {
        fillAuxiliary<InEvent>(eventTree().entryNumber());
        // back up, so event will not be skipped.
        eventTree().previous();
      }
      subRunAux().beginTime_ = eventAux().time();
      subRunAux().endTime_ = Timestamp::invalidTimestamp();
    }
    auto srp = std::make_shared<SubRunPrincipal>(subRunAux(),
                                                 processConfiguration_,
                                                 subRunTree().makeBranchMapper(),
                                                 subRunTree().makeDelayedReader(fileFormatVersion_,
                                                                                sqliteDB_,
                                                                                InSubRun,
                                                                                entryNumbers,
                                                                                fiIter_->eventID_),
                                                 secondaryFileNameIdx_ + 1,
                                                 primaryFile_->primarySRP_.get());

    subRunTree().fillGroups(*srp);
    if (!delayedReadSubRunProducts_) {
      srp->readImmediate();
    }
    primaryFile_->primarySRP_->addSecondaryPrincipal(static_pointer_cast<Principal>(srp));
    // FIXME: These secondary subRun principals will never be cached
    // FIXME: by the event processor!  This means that we cannot
    // FIXME: output subrun data products from them.
    primaryFile_->secondarySRPs_.emplace_back(move(srp));
    return true;
  }

  void
  RootInputFile::
  overrideRunNumber(RunID& id)
  {
    if (forcedRunOffset_ != 0) {
      id = RunID(id.run() + forcedRunOffset_);
    }
    if (id < RunID::firstRun()) {
      id = RunID::firstRun();
    }
  }

  void
  RootInputFile::
  overrideRunNumber(SubRunID& id)
  {
    if (forcedRunOffset_ != 0) {
      id = SubRunID(id.run() + forcedRunOffset_, id.subRun());
    }
  }

  void
  RootInputFile::
  overrideRunNumber(EventID& id, bool isRealData)
  {
    if (forcedRunOffset_ == 0) {
      return;
    }
    if (isRealData) {
      throw art::Exception{errors::Configuration, "RootInputFile::overrideRunNumber()"}
        << "The 'setRunNumber' parameter of RootInput cannot "
        << "be used with real data.\n";
    }
    id = EventID(id.run() + forcedRunOffset_, id.subRun(), id.event());
  }

  void
  RootInputFile::
  readEventHistoryTree()
  {
    // Read in the event history tree, if we have one...
    eventHistoryTree_ = static_cast<TTree*>(filePtr_->Get(rootNames::eventHistoryTreeName().c_str()));
    if (!eventHistoryTree_) {
      throw art::Exception{errors::DataCorruption}
        << "Failed to find the event history tree.\n";
    }
  }

  void
  RootInputFile::
  initializeDuplicateChecker()
  {
    if (duplicateChecker_.get() == nullptr) {
      return;
    }
    if (eventTree().next()) {
      fillAuxiliary<InEvent>(eventTree().entryNumber());
      duplicateChecker_->init(eventAux().isRealData(), fileIndex_);
    }
    eventTree().setEntryNumber(-1);
  }

  std::pair<RootInputFile::EntryNumbers,bool>
  RootInputFile::
  getEntryNumbers(BranchType const t)
  {
    EntryNumbers entries;
    auto it = fiIter_;
    if (it == fiEnd_)
      return std::pair<EntryNumbers,bool>{entries, true};

    auto const eid = it->eventID_;
    auto const subrun = eid.subRun();
    for (; it != fiEnd_ && eid == it->eventID_; ++it) {
      entries.push_back(it->entry_);
    }

    if (t == InEvent && entries.size() > 1ul) {
      throw Exception{errors::FileReadError}
        << "File " << file_ << " has multiple entries for\n"
        << eid << '\n';
    }

    bool const lastInSubRun = (it == fiEnd_ || it->eventID_.subRun() != subrun);
    return std::pair<EntryNumbers,bool>{entries, lastInSubRun};
  }

  void
  RootInputFile::
  fillPerBranchTypePresenceFlags(ProductList const& prodList)
  {
    for ( auto const& prodpr : prodList ){
      auto const& prod = prodpr.second;
      if (treePointers_[prod.branchType()]->hasBranch(prod.branchName())) {
        perBranchTypeProdPresence_[prod.branchType()].emplace( prod.branchID() );
      }
    }
  }


  void
  RootInputFile::
  dropOnInput(GroupSelectorRules const& rules, bool dropDescendants,
              bool /*dropMergeable*/, ProductList& prodList)
  {
    // This is the selector for drop on input.
    GroupSelector groupSelector;
    groupSelector.initialize(rules, prodList);
    // Do drop on input. On the first pass, just fill
    // in a set of branches to be dropped.
    set<BranchID> branchesToDrop;
    for (auto const& prod : prodList) {
      auto const& bd = prod.second;
      if (!groupSelector.selected(bd)) {
        if (dropDescendants) {
          branchChildren_->appendToDescendants(bd.branchID(), branchesToDrop);
        }
        else {
          branchesToDrop.insert(bd.branchID());
        }
      }
    }
    // On this pass, actually drop the branches.
    auto branchesToDropEnd = branchesToDrop.cend();
    for (auto I = prodList.begin(), E = prodList.end(); I != E;) {
      auto const& bd = I->second;
      bool drop = branchesToDrop.find(bd.branchID()) != branchesToDropEnd;
      if (!drop) {
        ++I;
        continue;
      }
      if (groupSelector.selected(bd)) {
        mf::LogWarning("RootInputFile")
          << "Branch '"
          << bd.branchName()
          << "' is being dropped from the input\n"
          << "of file '"
          << file_
          << "' because it is dependent on a branch\n"
          << "that was explicitly dropped.\n";
      }
      treePointers_[bd.branchType()]->dropBranch(bd.branchName());
      auto icopy = I++;
      prodList.erase(icopy);
    }
  }

  void
  RootInputFile::
  openSecondaryFile(int const idx)
  {
    secondaryFiles_[idx] =
      rifSequence_->openSecondaryFile(idx, secondaryFileNames_[idx], this);
  }

  std::unique_ptr<art::ResultsPrincipal>
  RootInputFile::
  readResults()
  {
    std::unique_ptr<art::ResultsPrincipal> resp;
    if (resultsTree()) {
      resultsTree().rewind();
      EntryNumbers const& entryNumbers {resultsTree().entryNumber()};
      assert(entryNumbers.size() == 1ull);
      fillAuxiliary<InResults>(entryNumbers.front());
      resp = std::make_unique<ResultsPrincipal>(resultsAux(),
                                                processConfiguration_,
                                                resultsTree().makeBranchMapper(),
                                                resultsTree().makeDelayedReader(fileFormatVersion_,
                                                                                InResults,
                                                                                entryNumbers,
                                                                                EventID{}),
                                                0,
                                                nullptr);
      resultsTree().fillGroups(*resp);
    } else { // Empty
      resp = std::make_unique<ResultsPrincipal>(ResultsAuxiliary{}, processConfiguration_);
    }
    return resp;
  }

} // namespace art
