#include "art/Framework/IO/Root/RootInputFile.h"

#include "Rtypes.h"
#include "TClass.h"
#include "TFile.h"
#include "TTree.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/GroupSelector.h"
#include "art/Framework/Core/MasterProductRegistry.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/IO/Root/DuplicateChecker.h"
#include "art/Framework/IO/Root/FastCloningInfoProvider.h"
#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/setMetaDataBranchAddress.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Provenance/BranchChildren.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/ParameterSetBlob.h"
#include "art/Persistency/Provenance/ParameterSetMap.h"
#include "art/Persistency/Provenance/ParentageRegistry.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/FriendlyName.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "fhiclcpp/make_ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <algorithm>
#include <utility>


using namespace cet;
using namespace std;


namespace art {

  RootInputFile::RootInputFile(string const& fileName,
                     string const& catalogName,
                     ProcessConfiguration const& processConfiguration,
                     string const& logicalFileName,
                     std::shared_ptr<TFile> filePtr,
                     EventID const &origEventID,
                     unsigned int eventsToSkip,
                     vector<SubRunID> const& whichSubRunsToSkip,
                     FastCloningInfoProvider const &fcip,
                     unsigned int treeCacheSize,
                     int treeMaxVirtualSize,
                     InputSource::ProcessingMode processingMode,
                     int forcedRunOffset,
                     vector<EventID> const& whichEventsToProcess,
                     bool noEventSort,
                     GroupSelectorRules const& groupSelectorRules,
                     bool dropMergeable,
                     std::shared_ptr<DuplicateChecker> duplicateChecker,
                     bool dropDescendants) :
      file_(fileName),
      logicalFile_(logicalFileName),
      catalog_(catalogName),
      processConfiguration_(processConfiguration),
      filePtr_(filePtr),
      fileFormatVersion_(),
      fileIndexSharedPtr_(new FileIndex),
      fileIndex_(*fileIndexSharedPtr_),
      fileIndexBegin_(fileIndex_.begin()),
      fileIndexEnd_(fileIndexBegin_),
      fileIndexIter_(fileIndexBegin_),
      origEventID_(origEventID),
      eventsToSkip_(eventsToSkip),
      whichSubRunsToSkip_(whichSubRunsToSkip),
      whichEventsToProcess_(whichEventsToProcess),
      eventListIter_(whichEventsToProcess_.begin()),
      noEventSort_(noEventSort),
      fastClonable_(false),
      eventAux_(),
      subRunAux_(),
      runAux_(),
      eventTree_(filePtr_, InEvent),
      subRunTree_(filePtr_, InSubRun),
      runTree_(filePtr_, InRun),
      treePointers_(),
      productListHolder_(),
      branchIDLists_(),
      processingMode_(processingMode),
      forcedRunOffset_(forcedRunOffset),
      eventHistoryTree_(0),
      history_(new History),
      branchChildren_(new BranchChildren),
      duplicateChecker_(duplicateChecker)
  {

    eventTree_.setCacheSize(treeCacheSize);

    eventTree_.setTreeMaxVirtualSize(treeMaxVirtualSize);
    subRunTree_.setTreeMaxVirtualSize(treeMaxVirtualSize);
    runTree_.setTreeMaxVirtualSize(treeMaxVirtualSize);

    treePointers_[InEvent] = &eventTree_;
    treePointers_[InSubRun]  = &subRunTree_;
    treePointers_[InRun]   = &runTree_;

    // Read the metadata tree.
    TTree *metaDataTree = dynamic_cast<TTree *>(filePtr_->Get(rootNames::metaDataTreeName().c_str()));
    if (!metaDataTree)
      throw art::Exception(errors::FileReadError) << "Could not find tree " << rootNames::metaDataTreeName()
                                                         << " in the input file.\n";

    FileFormatVersion *fftPtr = &fileFormatVersion_;
    setMetaDataBranchAddress(metaDataTree, fftPtr);

    FileIndex *findexPtr = &fileIndex_;
    setMetaDataBranchAddress(metaDataTree, findexPtr);

    ProductRegistry *ppList = 0;
    setMetaDataBranchAddress(metaDataTree, ppList);

    // TODO: update to separate tree per CMS code (2010/12/01).
    ParameterSetMap psetMap;
    ParameterSetMap *psetMapPtr = &psetMap;
    setMetaDataBranchAddress(metaDataTree, psetMapPtr);

    ProcessHistoryMap pHistMap;
    ProcessHistoryMap *pHistMapPtr = &pHistMap;
    setMetaDataBranchAddress(metaDataTree, pHistMapPtr);

    auto_ptr<BranchIDLists> branchIDListsAPtr(new BranchIDLists);
    BranchIDLists *branchIDListsPtr = branchIDListsAPtr.get();
    setMetaDataBranchAddress(metaDataTree, branchIDListsPtr);

    BranchChildren* branchChildrenBuffer = branchChildren_.get();
    setMetaDataBranchAddress(metaDataTree, branchChildrenBuffer);

    // Here we read the metadata tree
    input::getEntry(metaDataTree, 0);

    branchIDLists_.reset(branchIDListsAPtr.release());

    // Check the, "Era" of the input file (new since art v0.5.0). If it
    // does not match what we expect we cannot read the file. Required
    // since we reset the file versioning since forking off from
    // CMS. Files written by art prior to v0.5.0 will *also* not be
    // readable because they do not have this datum and because the run,
    // subrun and event-number handling has changed significantly.
    std::string const expected_era = art::getFileFormatEra();
    if (fileFormatVersion_.era_ != expected_era) {
       throw art::Exception(art::errors::FileReadError)
          << "Can only read files written during the \""
          << expected_era << "\" era: "
          << "Era of "
          << "\"" << file_
          << "\" was "
          << (fileFormatVersion_.era_.empty()?
              "not set":
              ("set to \"" + fileFormatVersion_.era_ + "\" "))
          << ".\n";
    }

    // Merge into the hashed registries.

    // Parameter Set
    for (ParameterSetMap::const_iterator i = psetMap.begin(), iEnd = psetMap.end(); i != iEnd; ++i) {
       fhicl::ParameterSet pset;
       fhicl::make_ParameterSet(i->second.pset_, pset);
      // Note ParameterSet::id() has the side effect of making sure the
      // parameter set *has* an ID.
      pset.id();
      fhicl::ParameterSetRegistry::put(pset);
    }
    ProcessHistoryRegistry::put(pHistMap);

    validateFile();

    // Read the parentage tree.  Old format files are handled internally in readParentageTree().
    readParentageTree();

    initializeDuplicateChecker();
    if (noEventSort_) fileIndex_.sortBy_Run_SubRun_EventEntry();
    fileIndexIter_ = fileIndexBegin_ = fileIndex_.begin();
    fileIndexEnd_ = fileIndex_.end();

    readEventHistoryTree();

    dropOnInput(groupSelectorRules, dropDescendants, dropMergeable, ppList->productList_);
    productListHolder_ = ppList;

    // Set up information from the product registry.
    ProductList const& prodList = ppList->productList_;
    for (ProductList::const_iterator
           it = prodList.begin(),
           itEnd = prodList.end();
         it != itEnd;
         ++it) {
      BranchDescription const& prod = it->second;
      treePointers_[prod.branchType()]->addBranch(it->first, prod, prod.branchName());
    }

    // Sort the EventID list the user supplied so that we can assume it is time ordered
    sort_all(whichEventsToProcess_);
    // Determine if this file is fast clonable.
    fastClonable_ = setIfFastClonable(fcip);

    reportOpened();
  }

  void
  RootInputFile::readParentageTree()
  {
    // New format file
    TTree* parentageTree = dynamic_cast<TTree*>(filePtr_->Get(rootNames::parentageTreeName().c_str()));
    if (!parentageTree)
      throw art::Exception(errors::FileReadError) << "Could not find tree " << rootNames::parentageTreeName()
                                                         << " in the input file.\n";

    ParentageID idBuffer;
    ParentageID* pidBuffer = &idBuffer;
    parentageTree->SetBranchAddress(rootNames::parentageIDBranchName().c_str(), &pidBuffer);

    Parentage parentageBuffer;
    Parentage *pParentageBuffer = &parentageBuffer;
    parentageTree->SetBranchAddress(rootNames::parentageBranchName().c_str(), &pParentageBuffer);

    for (Long64_t i = 0, numEntries = parentageTree->GetEntries(); i < numEntries; ++i) {
      input::getEntry(parentageTree, i);
      if (idBuffer != parentageBuffer.id())
        throw art::Exception(errors::DataCorruption) << "Corruption of Parentage tree detected.\n";
      ParentageRegistry::put(parentageBuffer);
    }
    parentageTree->SetBranchAddress(rootNames::parentageIDBranchName().c_str(), 0);
    parentageTree->SetBranchAddress(rootNames::parentageBranchName().c_str(), 0);
  }

  EventID RootInputFile::eventIDForFileIndexPosition() const {
    return (fileIndexIter_ == fileIndexEnd_)?
      EventID():
      fileIndexIter_->eventID_;
  }


  bool
  RootInputFile::setIfFastClonable(FastCloningInfoProvider const &fcip) const {
    if (!fcip.fastCloningPermitted()) return false;
    if (!fileFormatVersion_.fastCopyPossible()) return false;
    if (!fileIndex_.allEventsInEntryOrder()) return false;
    if (!whichEventsToProcess_.empty()) return false;
    if (eventsToSkip_ != 0) return false;
    if (fcip.remainingEvents() >= 0 && eventTree_.entries() > fcip.remainingEvents()) return false;
    if (fcip.remainingSubRuns() >= 0 && subRunTree_.entries() > fcip.remainingSubRuns()) return false;
    if (processingMode_ != InputSource::RunsSubRunsAndEvents) return false;
    if (forcedRunOffset_ != 0) return false;
    // Find entry for first event in file
    FileIndex::const_iterator it = fileIndexBegin_;
    while(it != fileIndexEnd_ && it->getEntryType() != FileIndex::kEvent) {
      ++it;
    }
    if (it == fileIndexEnd_) return false;
    if (it->eventID_ < origEventID_) return false;
    for (vector<SubRunID>::const_iterator it = whichSubRunsToSkip_.begin(),
          itEnd = whichSubRunsToSkip_.end(); it != itEnd; ++it) {
        if (fileIndex_.findSubRunPosition(*it, true) != fileIndexEnd_) {
          // We must skip a subRun in this file.  We will simply assume that
          // it may contain an event, in which case we cannot fast copy.
          return false;
        }
    }
    return true;
  }


  int
  RootInputFile::setForcedRunOffset(RunNumber_t const& forcedRunNumber) {
     if (fileIndexBegin_ == fileIndexEnd_) return 0;
     forcedRunOffset_ = (RunID(forcedRunNumber).isValid())?(forcedRunNumber - fileIndexBegin_->eventID_.run()):0;
     if (forcedRunOffset_ != 0) {
        fastClonable_ = false;
     }
     return forcedRunOffset_;
  }

  std::shared_ptr<FileBlock>
  RootInputFile::createFileBlock() const {
    return std::shared_ptr<FileBlock>(new FileBlock(fileFormatVersion_,
                                                     eventTree_.tree(),
                                                     eventTree_.metaTree(),
                                                     subRunTree_.tree(),
                                                     subRunTree_.metaTree(),
                                                     runTree_.tree(),
                                                     runTree_.metaTree(),
                                                     fastClonable(),
                                                     file_,
                                                     branchChildren_));
  }

  FileIndex::EntryType
  RootInputFile::getEntryType() const {
    if (fileIndexIter_ == fileIndexEnd_) {
      return FileIndex::kEnd;
    }
    return fileIndexIter_->getEntryType();
  }

  // Temporary KLUDGE until we can properly merge runs and subRuns across files
  // This KLUDGE skips duplicate run or subRun entries.
  FileIndex::EntryType
  RootInputFile::getEntryTypeSkippingDups() {
    if (fileIndexIter_ == fileIndexEnd_) {
      return FileIndex::kEnd;
    }
    if ((!fileIndexIter_->eventID_.isValid()) && fileIndexIter_ != fileIndexBegin_) {
       if ((fileIndexIter_-1)->eventID_.subRun() == fileIndexIter_->eventID_.subRun()) {
        ++fileIndexIter_;
        return getEntryTypeSkippingDups();
      }
    }
    return fileIndexIter_->getEntryType();
  }

  FileIndex::EntryType
  RootInputFile::getNextEntryTypeWanted() {
     bool specifiedEvents = !whichEventsToProcess_.empty();
     if (specifiedEvents && eventListIter_ == whichEventsToProcess_.end()) {
        // We are processing specified events, and we are done with them.
        fileIndexIter_ = fileIndexEnd_;
        return FileIndex::kEnd;
     }
     FileIndex::EntryType entryType = getEntryTypeSkippingDups();
     if (entryType == FileIndex::kEnd) {
        return FileIndex::kEnd;
     }
     RunID currentRun(fileIndexIter_->eventID_.runID());
     if (!currentRun.isValid()) return FileIndex::kEnd;
     if (specifiedEvents) {
        // We are processing specified events.
        if (currentRun > eventListIter_->runID()) {
           // The next specified event is in a run not in the file or already passed.  Skip the event
           ++eventListIter_;
           return getNextEntryTypeWanted();
        }
        // Skip any runs before the next specified event.
        if (currentRun < eventListIter_->runID()) {
           fileIndexIter_ = fileIndex_.findRunPosition(eventListIter_->runID(), false);
           return getNextEntryTypeWanted();
        }
     }
     if (entryType == FileIndex::kRun) {
        // Skip any runs before the first run specified
        if (currentRun < origEventID_.runID()) {
           fileIndexIter_ = fileIndex_.findRunPosition(origEventID_.runID(), false);
           return getNextEntryTypeWanted();
        }
        return FileIndex::kRun;
     } else if (processingMode_ == InputSource::Runs) {
        fileIndexIter_ = fileIndex_.findRunPosition(currentRun.isValid()?currentRun.next():currentRun, false);
        return getNextEntryTypeWanted();
     }
     SubRunID const& currentSubRun = fileIndexIter_->eventID_.subRunID();
     if (specifiedEvents) {
        // We are processing specified events.
        assert (currentRun == eventListIter_->runID());
        // Get the subRun number of the next specified event.
        FileIndex::const_iterator iter = fileIndex_.findEventPosition(*eventListIter_, true);
        if (iter == fileIndexEnd_ || currentSubRun > iter->eventID_.subRunID()) {
           // Event Not Found or already passed. Skip the next specified event;
           ++eventListIter_;
           return getNextEntryTypeWanted();
        }
        // Skip any subRuns before the next specified event.
        if (currentSubRun < iter->eventID_.subRunID()) {
           fileIndexIter_ = fileIndex_.findPosition(EventID::invalidEvent(iter->eventID_.subRunID()));
           return getNextEntryTypeWanted();
        }
     }
     if (entryType == FileIndex::kSubRun) {
        // Skip any subRuns before the first subRun specified
        if (currentRun == origEventID_.runID() &&
            currentSubRun < origEventID_.subRunID()) {
           fileIndexIter_ = fileIndex_.findSubRunOrRunPosition(origEventID_.subRunID());
           return getNextEntryTypeWanted();
        }
        // Skip the subRun if it is in whichSubRunsToSkip_.
        if (binary_search_all(whichSubRunsToSkip_, currentSubRun)) {
           fileIndexIter_ = fileIndex_.findSubRunOrRunPosition(currentSubRun.next());
           return getNextEntryTypeWanted();
        }
        return FileIndex::kSubRun;
     } else if (processingMode_ == InputSource::RunsAndSubRuns) {
        fileIndexIter_ = fileIndex_.findSubRunOrRunPosition(currentSubRun.next());
        return getNextEntryTypeWanted();
     }
     assert (entryType == FileIndex::kEvent);
     // Skip any events before the first event specified
     if (fileIndexIter_->eventID_ < origEventID_) {
        fileIndexIter_ = fileIndex_.findPosition(origEventID_);
        return getNextEntryTypeWanted();
     }
     if (specifiedEvents) {
        // We have specified events to process and we've already positioned the file
        // to execute the run and subRun entry for the current event in the list.
        // Just position to the right event.
        fileIndexIter_ = fileIndex_.findEventPosition(*eventListIter_,
                                                      false);
        if (fileIndexIter_->eventID_ != *eventListIter_) {
           // Event was not found.
           ++eventListIter_;
           return getNextEntryTypeWanted();
        }
        // Event was found.
        // For the next time around move to the next specified event
        ++eventListIter_;

        if (duplicateChecker_.get() != 0 &&
            duplicateChecker_->isDuplicateAndCheckActive(fileIndexIter_->eventID_,
                                                         file_)) {
           ++fileIndexIter_;
           return getNextEntryTypeWanted();
        }

        if (eventsToSkip_ != 0) {
           // We have specified a count of events to skip.  So decrement the count and skip this event.
           --eventsToSkip_;
           return getNextEntryTypeWanted();
        }

        return FileIndex::kEvent;
     }

     if (duplicateChecker_.get() != 0 &&
         duplicateChecker_->isDuplicateAndCheckActive(fileIndexIter_->eventID_,
                                                      file_)) {
        ++fileIndexIter_;
        return getNextEntryTypeWanted();
     }

     if (eventsToSkip_ != 0) {
        // We have specified a count of events to skip, keep skipping events in this subRun block
        // until we reach the end of the subRun block or the full count of the number of events to skip.
        while (eventsToSkip_ != 0 && fileIndexIter_ != fileIndexEnd_ &&
               getEntryTypeSkippingDups() == FileIndex::kEvent) {
           ++fileIndexIter_;
           --eventsToSkip_;

           while (
                  eventsToSkip_ != 0 &&
                  fileIndexIter_ != fileIndexEnd_ &&
                  fileIndexIter_->getEntryType() == FileIndex::kEvent &&
                  duplicateChecker_.get() != 0 &&
                  duplicateChecker_->isDuplicateAndCheckActive(fileIndexIter_->eventID_,
                                                               file_)) {
              ++fileIndexIter_;
           }
        }
        return getNextEntryTypeWanted();
     }
     return FileIndex::kEvent;
  }

  void
  RootInputFile::validateFile() {
    if (!fileFormatVersion_.isValid()) {
      fileFormatVersion_.value_ = 0;
    }
    if(!eventTree_.isValid()) {
      throw art::Exception(errors::DataCorruption) <<
         "'Events' tree is corrupted or not present\n" << "in the input file.\n";
    }
    if (fileIndex_.empty()) {
       throw art::Exception(art::errors::FileReadError)
          << "FileIndex information is missing for the input file.\n";
    }
  }

  void
  RootInputFile::reportOpened() {
    // Report file opened.
    string const label = "source";
    string moduleName = "RootInput";
  }

  void
  RootInputFile::close(bool reallyClose) {
    if (reallyClose) {
      filePtr_->Close();
    }
  }

  void
  RootInputFile::fillEventAuxiliary() {
     EventAuxiliary *pEvAux = &eventAux_;
     eventTree_.fillAux<EventAuxiliary>(pEvAux);
  }

  void
  RootInputFile::fillHistory() {
     // We could consider doing delayed reading, but because we have to
     // store this History object in a different tree than the event
     // data tree, this is too hard to do in this first version.

     History* pHistory = history_.get();
     TBranch* eventHistoryBranch = eventHistoryTree_->GetBranch(rootNames::eventHistoryBranchName().c_str());
     if (!eventHistoryBranch)
        throw art::Exception(errors::DataCorruption)
           << "Failed to find history branch in event history tree.\n";
     eventHistoryBranch->SetAddress(&pHistory);
     input::getEntry(eventHistoryTree_, eventTree_.entryNumber());
  }

  void
  RootInputFile::fillSubRunAuxiliary() {
     SubRunAuxiliary *pSubRunAux = &subRunAux_;
     subRunTree_.fillAux<SubRunAuxiliary>(pSubRunAux);
  }

  void
  RootInputFile::fillRunAuxiliary() {
     RunAuxiliary *pRunAux = &runAux_;
     runTree_.fillAux<RunAuxiliary>(pRunAux);
  }

  int
  RootInputFile::skipEvents(int offset) {
    while (offset > 0 && fileIndexIter_ != fileIndexEnd_) {
      if (fileIndexIter_->getEntryType() == FileIndex::kEvent) {
        --offset;
      }
      ++fileIndexIter_;
    }
    while (offset < 0 && fileIndexIter_ != fileIndexBegin_) {
      --fileIndexIter_;
      if (fileIndexIter_->getEntryType() == FileIndex::kEvent) {
        ++offset;
      }
    }
    while (fileIndexIter_ != fileIndexEnd_ && fileIndexIter_->getEntryType() != FileIndex::kEvent) {
      ++fileIndexIter_;
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
  // We do *not* create the EDProduct instance (the equivalent of reading
  // the branch containing this EDProduct. That will be done by the Delayed Reader,
  //  when it is asked to do so.
  //
  auto_ptr<EventPrincipal>
  RootInputFile::readEvent() {
    assert(fileIndexIter_ != fileIndexEnd_);
    assert(fileIndexIter_->getEntryType() == FileIndex::kEvent);
    assert(fileIndexIter_->eventID_.runID().isValid());
    // Set the entry in the tree, and read the event at that entry.
    eventTree_.setEntryNumber(fileIndexIter_->entry_);
    auto_ptr<EventPrincipal> ep = readCurrentEvent();

    assert(ep.get() != 0);
    assert(eventAux_.run() == fileIndexIter_->eventID_.run() + forcedRunOffset_);
    assert(eventAux_.subRunID() == fileIndexIter_->eventID_.subRunID());

    // report event read from file
    ++fileIndexIter_;
    return ep;
  }

  // Reads event at the current entry in the tree.
  // Note: This function neither uses nor sets fileIndexIter_.
  auto_ptr<EventPrincipal>
  RootInputFile::readCurrentEvent() {
    if (!eventTree_.current()) {
      return auto_ptr<EventPrincipal>(0);
    }
    fillEventAuxiliary();
    fillHistory();
    // FIXME: move this functionality into fillEventAuxiliary to avoid
    // the const_cast.
    overrideRunNumber(const_cast<EventID&>(eventAux_.id()), eventAux_.isRealData());

    // We're not done ... so prepare the EventPrincipal
    auto_ptr<EventPrincipal> thisEvent(new EventPrincipal(
                eventAux_,
                processConfiguration_,
                history_,
                eventTree_.makeBranchMapper(),
                eventTree_.makeDelayedReader(false)));

    // Create a group in the event for each product
    eventTree_.fillGroups(*thisEvent);
    return thisEvent;
  }

  void
  RootInputFile::setAtEventEntry(FileIndex::EntryNumber_t entry) {
    eventTree_.setEntryNumber(entry);
  }

  std::shared_ptr<RunPrincipal>
  RootInputFile::readRun() {
    assert(fileIndexIter_ != fileIndexEnd_);
    assert(fileIndexIter_->getEntryType() == FileIndex::kRun);
    runTree_.setEntryNumber(fileIndexIter_->entry_);
    fillRunAuxiliary();
    assert(runAux_.id() == fileIndexIter_->eventID_.runID());
    overrideRunNumber(runAux_.id_);
    if (runAux_.beginTime() == Timestamp::invalidTimestamp()) {
      // RunAuxiliary did not contain a valid timestamp.  Take it from the next event.
      if (eventTree_.next()) {
        fillEventAuxiliary();
        // back up, so event will not be skipped.
        eventTree_.previous();
      }
      runAux_.beginTime_ = eventAux_.time();
      runAux_.endTime_ = Timestamp::invalidTimestamp();
    }
    std::shared_ptr<RunPrincipal> thisRun(
        new RunPrincipal(runAux_,
                         processConfiguration_,
                         runTree_.makeBranchMapper(),
                         runTree_.makeDelayedReader()));
    // Create a group in the run for each product
    runTree_.fillGroups(*thisRun);
    // Read in all the products now.
    thisRun->readImmediate();
    ++fileIndexIter_;
    return thisRun;
  }

  std::shared_ptr<SubRunPrincipal>
  RootInputFile::readSubRun(std::shared_ptr<RunPrincipal> rp) {
    assert(fileIndexIter_ != fileIndexEnd_);
    assert(fileIndexIter_->getEntryType() == FileIndex::kSubRun);
    subRunTree_.setEntryNumber(fileIndexIter_->entry_);
    fillSubRunAuxiliary();
    assert(subRunAux_.id() == fileIndexIter_->eventID_.subRunID());
    overrideRunNumber(subRunAux_.id_);
    assert(subRunAux_.runID() == rp->id());

    if (subRunAux_.beginTime() == Timestamp::invalidTimestamp()) {
      // SubRunAuxiliary did not contain a timestamp. Take it from the next event.
      if (eventTree_.next()) {
        fillEventAuxiliary();
        // back up, so event will not be skipped.
        eventTree_.previous();
      }
      subRunAux_.beginTime_ = eventAux_.time();
      subRunAux_.endTime_ = Timestamp::invalidTimestamp();
    }
    std::shared_ptr<SubRunPrincipal> thisSubRun(
        new SubRunPrincipal(subRunAux_,
                            processConfiguration_,
                            subRunTree_.makeBranchMapper(),
                            subRunTree_.makeDelayedReader()));
    // Create a group in the subRun for each product
    subRunTree_.fillGroups(*thisSubRun);
    // Read in all the products now.
    thisSubRun->readImmediate();
    ++fileIndexIter_;
    return thisSubRun;
  }

  bool
  RootInputFile::setEntryAtEvent(EventID const &eID, bool exact) {
    fileIndexIter_ = fileIndex_.findEventPosition(eID, exact);
    if (fileIndexIter_ == fileIndexEnd_) return false;
    eventTree_.setEntryNumber(fileIndexIter_->entry_);
    return true;
  }

  bool
  RootInputFile::setEntryAtSubRun(SubRunID const& subRun) {
    fileIndexIter_ = fileIndex_.findSubRunPosition(subRun, true);
    if (fileIndexIter_ == fileIndexEnd_) return false;
    subRunTree_.setEntryNumber(fileIndexIter_->entry_);
    return true;
  }

  bool
  RootInputFile::setEntryAtRun(RunID const& run) {
    fileIndexIter_ = fileIndex_.findRunPosition(run, true);
    if (fileIndexIter_ == fileIndexEnd_) return false;
    runTree_.setEntryNumber(fileIndexIter_->entry_);
    return true;
  }

  void
  RootInputFile::overrideRunNumber(RunID & id) {
    if (forcedRunOffset_ != 0) {
      id = RunID(id.run() + forcedRunOffset_);
    }
    if (id < RunID::firstRun()) id = RunID::firstRun();
  }

  void
  RootInputFile::overrideRunNumber(SubRunID & id) {
    if (forcedRunOffset_ != 0) {
      id = SubRunID(id.run() + forcedRunOffset_, id.subRun());
    }
  }

  void
  RootInputFile::overrideRunNumber(EventID & id, bool isRealData) {
    if (forcedRunOffset_ != 0) {
      if (isRealData) {
        throw art::Exception(errors::Configuration,"RootInputFile::RootInputFile()")
          << "The 'setRunNumber' parameter of RootInput cannot be used with real data.\n";
      }
      id = EventID(id.run() + forcedRunOffset_, id.subRun(), id.event());
    }
  }

  void
  RootInputFile::readEventHistoryTree() {
    // Read in the event history tree, if we have one...
    eventHistoryTree_ = dynamic_cast<TTree*>(filePtr_->Get(rootNames::eventHistoryTreeName().c_str()));

    if (!eventHistoryTree_)
      throw art::Exception(errors::DataCorruption)
        << "Failed to find the event history tree.\n";
  }

  void
  RootInputFile::initializeDuplicateChecker() {
    if (duplicateChecker_.get() != 0) {
      if (eventTree_.next()) {
        fillEventAuxiliary();
        duplicateChecker_->init(eventAux_.isRealData(),
                                fileIndex_);
      }
      eventTree_.setEntryNumber(-1);
    }
  }

  void
  RootInputFile::dropOnInput(GroupSelectorRules const& rules,
                             bool dropDescendants, 
                             bool dropMergeable, 
                             ProductList &branchDescriptions) {
    // This is the selector for drop on input.
    GroupSelector groupSelector;
    groupSelector.initialize(rules, branchDescriptions);

    // Do drop on input. On the first pass, just fill in a set of branches to be dropped.
    set<BranchID> branchesToDrop;
    for (ProductList::const_iterator
           it = branchDescriptions.begin(),
           itEnd = branchDescriptions.end();
         it != itEnd;
         ++it) {
      BranchDescription const& prod = it->second;
      if (!groupSelector.selected(prod)) {
        if (dropDescendants) {
          branchChildren_->appendToDescendants(prod.branchID(), branchesToDrop);
        } else {
          branchesToDrop.insert(prod.branchID());
        }
      }
    }

    // On this pass, actually drop the branches.
    set<BranchID>::const_iterator branchesToDropEnd = branchesToDrop.end();
    for (ProductList::iterator
           it = branchDescriptions.begin(),
           itEnd = branchDescriptions.end();
         it != itEnd;
         ) {
      BranchDescription const& prod = it->second;
      bool drop = branchesToDrop.find(prod.branchID()) != branchesToDropEnd;
      if (drop) {
        if (groupSelector.selected(prod)) {
          mf::LogWarning("RootInputFile")
            << "Branch '" << prod.branchName() << "' is being dropped from the input\n"
            << "of file '" << file_ << "' because it is dependent on a branch\n"
            << "that was explicitly dropped.\n";
        }
        treePointers_[prod.branchType()]->dropBranch(prod.branchName());
        ProductList::iterator icopy = it;
        ++it;
        branchDescriptions.erase(icopy);
      } else {
        ++it;
      }
    }

    // Drop on input mergeable run and subRun products, this needs to be invoked for
    // secondary file input
    if (dropMergeable) {
      for (ProductList::iterator
             it = branchDescriptions.begin(),
             itEnd = branchDescriptions.end();
           it != itEnd;
           ) {
        BranchDescription const& prod = it->second;
        if (prod.branchType() != InEvent) {
          TClass *cp = TClass::GetClass(prod.wrappedName().c_str());
          std::shared_ptr<EDProduct> dummy(static_cast<EDProduct *>(cp->New()));
          if (dummy->isMergeable()) {
            treePointers_[prod.branchType()]->dropBranch(prod.branchName());
            ProductList::iterator icopy = it;
            ++it;
            branchDescriptions.erase(icopy);
          } else {
            ++it;
          }
        }
        else ++it;
      }
    }
  }

}  // art
