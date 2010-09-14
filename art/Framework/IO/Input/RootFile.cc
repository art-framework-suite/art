#include "art/Framework/IO/Input/RootFile.h"

#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/GroupSelector.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/IO/Input/DuplicateChecker.h"
#include "art/Framework/IO/Input/ProvenanceAdaptor.h"
#include "art/Framework/Services/Registry/Service.h"
#include "art/ParameterSet/Registry.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/RefCoreStreamer.h"
#include "art/Persistency/Provenance/BranchChildren.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/ModuleDescriptionRegistry.h"
#include "art/Persistency/Provenance/ParameterSetBlob.h"
#include "art/Persistency/Provenance/ParentageRegistry.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Utilities/Algorithms.h"
#include "art/Utilities/EDMException.h"
#include "art/Utilities/FriendlyName.h"
#include "art/Utilities/GlobalIdentifier.h"
#include "fhiclcpp/ParameterSet.h"

//used for backward compatibility
#include "art/Persistency/Provenance/BranchEntryDescription.h"
#include "art/Persistency/Provenance/EntryDescriptionRegistry.h"
#include "art/Persistency/Provenance/EventAux.h"
#include "art/Persistency/Provenance/RunAux.h"
#include "art/Persistency/Provenance/RunSubRunEntryInfo.h"
#include "art/Persistency/Provenance/SubRunAux.h"

#include "Rtypes.h"
#include "TClass.h"
#include "TFile.h"
#include "TROOT.h"
#include "TTree.h"

#include "MessageFacility/MessageLogger.h"

#include <algorithm>


namespace edm {

  RootFile::RootFile(std::string const& fileName,
                     std::string const& catalogName,
                     ProcessConfiguration const& processConfiguration,
                     std::string const& logicalFileName,
                     boost::shared_ptr<TFile> filePtr,
                     RunNumber_t const& startAtRun,
                     SubRunNumber_t const& startAtSubRun,
                     EventNumber_t const& startAtEvent,
                     unsigned int eventsToSkip,
                     std::vector<SubRunID> const& whichSubRunsToSkip,
                     int remainingEvents,
                     int remainingSubRuns,
                     unsigned int treeCacheSize,
                     int treeMaxVirtualSize,
                     InputSource::ProcessingMode processingMode,
                     int forcedRunOffset,
                     std::vector<EventID> const& whichEventsToProcess,
                     bool noEventSort,
                     GroupSelectorRules const& groupSelectorRules,
                     bool dropMergeable,
                     boost::shared_ptr<DuplicateChecker> duplicateChecker,
                     bool dropDescendants) :
      file_(fileName),
      logicalFile_(logicalFileName),
      catalog_(catalogName),
      processConfiguration_(processConfiguration),
      filePtr_(filePtr),
      fileFormatVersion_(),
      fid_(),
      fileIndexSharedPtr_(new FileIndex),
      fileIndex_(*fileIndexSharedPtr_),
      fileIndexBegin_(fileIndex_.begin()),
      fileIndexEnd_(fileIndexBegin_),
      fileIndexIter_(fileIndexBegin_),
      eventProcessHistoryIDs_(),
      eventProcessHistoryIter_(eventProcessHistoryIDs_.begin()),
      startAtRun_(startAtRun),
      startAtSubRun_(startAtSubRun),
      startAtEvent_(startAtEvent),
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
      productRegistry_(),
      branchIDLists_(),
      processingMode_(processingMode),
      forcedRunOffset_(forcedRunOffset),
      newBranchToOldBranch_(),
      eventHistoryTree_(0),
      history_(new History),
      branchChildren_(new BranchChildren),
      duplicateChecker_(duplicateChecker),
      provenanceAdaptor_() {

    eventTree_.setCacheSize(treeCacheSize);

    eventTree_.setTreeMaxVirtualSize(treeMaxVirtualSize);
    subRunTree_.setTreeMaxVirtualSize(treeMaxVirtualSize);
    runTree_.setTreeMaxVirtualSize(treeMaxVirtualSize);

    treePointers_[InEvent] = &eventTree_;
    treePointers_[InSubRun]  = &subRunTree_;
    treePointers_[InRun]   = &runTree_;

    setRefCoreStreamer(0, true); // backward compatibility

    // Read the metadata tree.
    TTree *metaDataTree = dynamic_cast<TTree *>(filePtr_->Get(poolNames::metaDataTreeName().c_str()));
    if (!metaDataTree)
      throw edm::Exception(errors::FileReadError) << "Could not find tree " << poolNames::metaDataTreeName()
                                                         << " in the input file.\n";

    // To keep things simple, we just read in every possible branch that exists.
    // We don't pay attention to which branches exist in which file format versions

    FileFormatVersion *fftPtr = &fileFormatVersion_;
    metaDataTree->SetBranchAddress(poolNames::fileFormatVersionBranchName().c_str(), &fftPtr);

    FileID *fidPtr = &fid_;
    if (metaDataTree->FindBranch(poolNames::fileIdentifierBranchName().c_str()) != 0) {
      metaDataTree->SetBranchAddress(poolNames::fileIdentifierBranchName().c_str(), &fidPtr);
    }

    FileIndex *findexPtr = &fileIndex_;
    if (metaDataTree->FindBranch(poolNames::fileIndexBranchName().c_str()) != 0) {
      metaDataTree->SetBranchAddress(poolNames::fileIndexBranchName().c_str(), &findexPtr);
    }

    // Need to read to a temporary registry so we can do a translation of the BranchKeys.
    // This preserves backward compatibility against friendly class name algorithm changes.
    ProductRegistry tempReg;
    ProductRegistry *ppReg = &tempReg;
    metaDataTree->SetBranchAddress(poolNames::productDescriptionBranchName().c_str(),(&ppReg));

    typedef std::map<ParameterSetID, ParameterSetBlob> PsetMap;
    PsetMap psetMap;
    PsetMap *psetMapPtr = &psetMap;
    metaDataTree->SetBranchAddress(poolNames::parameterSetMapBranchName().c_str(), &psetMapPtr);

    ProcessHistoryRegistry::collection_type pHistMap;
    ProcessHistoryRegistry::collection_type *pHistMapPtr = &pHistMap;
    metaDataTree->SetBranchAddress(poolNames::processHistoryMapBranchName().c_str(), &pHistMapPtr);

    std::auto_ptr<BranchIDListRegistry::collection_type> branchIDListsAPtr(new BranchIDListRegistry::collection_type);
    BranchIDListRegistry::collection_type *branchIDListsPtr = branchIDListsAPtr.get();
    if (metaDataTree->FindBranch(poolNames::branchIDListBranchName().c_str()) != 0) {
      metaDataTree->SetBranchAddress(poolNames::branchIDListBranchName().c_str(), &branchIDListsPtr);
    }

    BranchChildren* branchChildrenBuffer = branchChildren_.get();
    if (metaDataTree->FindBranch(poolNames::productDependenciesBranchName().c_str()) != 0) {
      metaDataTree->SetBranchAddress(poolNames::productDependenciesBranchName().c_str(), &branchChildrenBuffer);
    }

    // backward compatibility
    std::vector<EventProcessHistoryID> *eventHistoryIDsPtr = &eventProcessHistoryIDs_;
    if (metaDataTree->FindBranch(poolNames::eventHistoryBranchName().c_str()) != 0) {
      metaDataTree->SetBranchAddress(poolNames::eventHistoryBranchName().c_str(), &eventHistoryIDsPtr);
    }

    ModuleDescriptionRegistry::collection_type mdMap;
    ModuleDescriptionRegistry::collection_type *mdMapPtr = &mdMap;
    if (metaDataTree->FindBranch(poolNames::moduleDescriptionMapBranchName().c_str()) != 0) {
      metaDataTree->SetBranchAddress(poolNames::moduleDescriptionMapBranchName().c_str(), &mdMapPtr);
    }
    // Here we read the metadata tree
    input::getEntry(metaDataTree, 0);

    setRefCoreStreamer(true);  // backward compatibility

    if (fileFormatVersion_.value_ < 11) {
      // Old format input file.  Create a provenance adaptor.
      provenanceAdaptor_.reset(new ProvenanceAdaptor(tempReg, pHistMap, psetMap, mdMap));
      // Fill in the branchIDLists branch from the provenance adaptor
      branchIDLists_ = provenanceAdaptor_->branchIDLists();
    } else {
      // New format input file. The branchIDLists branch was read directly from the input file.
      if (metaDataTree->FindBranch(poolNames::branchIDListBranchName().c_str()) == 0) {
        throw edm::Exception(errors::EventCorruption)
          << "Failed to find branchIDLists branch in metaData tree.\n";
      }
      branchIDLists_.reset(branchIDListsAPtr.release());
    }

    // Merge into the hashed registries.
    pset::Registry& psetRegistry = *pset::Registry::instance();
    for (PsetMap::const_iterator i = psetMap.begin(), iEnd = psetMap.end(); i != iEnd; ++i) {
      fhicl::ParameterSet pset(i->second.pset_);
      pset.setID(i->first);
      psetRegistry.insertMapped(pset);
    }
    ProcessHistoryRegistry::instance()->insertCollection(pHistMap);
    ModuleDescriptionRegistry::instance()->insertCollection(mdMap);

    validateFile();

    // Read the parentage tree.  Old format files are handled internally in readParentageTree().
    readParentageTree();

    initializeDuplicateChecker();
    if (noEventSort_) fileIndex_.sortBy_Run_SubRun_EventEntry();
    fileIndexIter_ = fileIndexBegin_ = fileIndex_.begin();
    fileIndexEnd_ = fileIndex_.end();
    eventProcessHistoryIter_ = eventProcessHistoryIDs_.begin();

    readEventHistoryTree();

    // Set product presence information in the product registry.
    ProductRegistry::ProductList const& pList = tempReg.productList();
    for (ProductRegistry::ProductList::const_iterator it = pList.begin(), itEnd = pList.end();
        it != itEnd; ++it) {
      BranchDescription const& prod = it->second;
      treePointers_[prod.branchType()]->setPresence(prod);
    }

    // freeze our temporary product registry
    tempReg.setFrozen();

    std::auto_ptr<ProductRegistry> newReg(new ProductRegistry);

    // Do the translation from the old registry to the new one
    {
      ProductRegistry::ProductList const& prodList = tempReg.productList();
      for (ProductRegistry::ProductList::const_iterator it = prodList.begin(), itEnd = prodList.end();
           it != itEnd; ++it) {
        BranchDescription const& prod = it->second;
        std::string newFriendlyName = friendlyname::friendlyName(prod.className());
        if (newFriendlyName == prod.friendlyClassName()) {
          newReg->copyProduct(prod);
        } else {
          if (fileFormatVersion_.value_ >= 11) {
            throw edm::Exception(errors::UnimplementedFeature)
              << "Cannot change friendly class name algorithm without more development work\n"
              << "to update BranchIDLists.  Contact the framework group.\n";
          }
          BranchDescription newBD(prod);
          newBD.updateFriendlyClassName();
          newReg->copyProduct(newBD);
          // Need to call init to get old branch name.
          prod.init();
          newBranchToOldBranch_.insert(std::make_pair(newBD.branchName(), prod.branchName()));
        }
      }
      // freeze the product registry
      newReg->setFrozen();
      productRegistry_.reset(newReg.release());
    }

    dropOnInput(groupSelectorRules, dropDescendants, dropMergeable);

    // Set up information from the product registry.
    ProductRegistry::ProductList const& prodList = productRegistry()->productList();
    for (ProductRegistry::ProductList::const_iterator it = prodList.begin(), itEnd = prodList.end();
        it != itEnd; ++it) {
      BranchDescription const& prod = it->second;
      treePointers_[prod.branchType()]->addBranch(it->first, prod,
                                                  newBranchToOldBranch(prod.branchName()));
    }

    // Sort the EventID list the user supplied so that we can assume it is time ordered
    sort_all(whichEventsToProcess_);
    // Determine if this file is fast clonable.
    fastClonable_ = setIfFastClonable(remainingEvents, remainingSubRuns);

    reportOpened();
  }

  void
  RootFile::readEntryDescriptionTree() {
    // Called only for old format files.
    if (fileFormatVersion_.value_ < 8) return;
    TTree* entryDescriptionTree = dynamic_cast<TTree*>(filePtr_->Get(poolNames::entryDescriptionTreeName().c_str()));
    if (!entryDescriptionTree)
      throw edm::Exception(errors::FileReadError) << "Could not find tree " << poolNames::entryDescriptionTreeName()
                                                         << " in the input file.\n";


    EntryDescriptionID idBuffer;
    EntryDescriptionID* pidBuffer = &idBuffer;
    entryDescriptionTree->SetBranchAddress(poolNames::entryDescriptionIDBranchName().c_str(), &pidBuffer);

    EntryDescriptionRegistry& oldregistry = *EntryDescriptionRegistry::instance();

    EventEntryDescription entryDescriptionBuffer;
    EventEntryDescription *pEntryDescriptionBuffer = &entryDescriptionBuffer;
    entryDescriptionTree->SetBranchAddress(poolNames::entryDescriptionBranchName().c_str(), &pEntryDescriptionBuffer);

    // Fill in the parentage registry.
    ParentageRegistry& registry = *ParentageRegistry::instance();

    for (Long64_t i = 0, numEntries = entryDescriptionTree->GetEntries(); i < numEntries; ++i) {
      input::getEntry(entryDescriptionTree, i);
      if (idBuffer != entryDescriptionBuffer.id())
        throw edm::Exception(errors::EventCorruption) << "Corruption of EntryDescription tree detected.\n";
      oldregistry.insertMapped(entryDescriptionBuffer);
      Parentage parents;
      parents.parents() = entryDescriptionBuffer.parents();
      registry.insertMapped(parents);
    }
    entryDescriptionTree->SetBranchAddress(poolNames::entryDescriptionIDBranchName().c_str(), 0);
    entryDescriptionTree->SetBranchAddress(poolNames::entryDescriptionBranchName().c_str(), 0);
  }

  void
  RootFile::readParentageTree()
  {
    if (fileFormatVersion_.value_ < 11) {
      // Old format file.
      readEntryDescriptionTree();
      return;
    }
    // New format file
    TTree* parentageTree = dynamic_cast<TTree*>(filePtr_->Get(poolNames::parentageTreeName().c_str()));
    if (!parentageTree)
      throw edm::Exception(errors::FileReadError) << "Could not find tree " << poolNames::parentageTreeName()
                                                         << " in the input file.\n";

    ParentageID idBuffer;
    ParentageID* pidBuffer = &idBuffer;
    parentageTree->SetBranchAddress(poolNames::parentageIDBranchName().c_str(), &pidBuffer);

    Parentage parentageBuffer;
    Parentage *pParentageBuffer = &parentageBuffer;
    parentageTree->SetBranchAddress(poolNames::parentageBranchName().c_str(), &pParentageBuffer);

    ParentageRegistry& registry = *ParentageRegistry::instance();

    for (Long64_t i = 0, numEntries = parentageTree->GetEntries(); i < numEntries; ++i) {
      input::getEntry(parentageTree, i);
      if (idBuffer != parentageBuffer.id())
        throw edm::Exception(errors::EventCorruption) << "Corruption of Parentage tree detected.\n";
      registry.insertMapped(parentageBuffer);
    }
    parentageTree->SetBranchAddress(poolNames::parentageIDBranchName().c_str(), 0);
    parentageTree->SetBranchAddress(poolNames::parentageBranchName().c_str(), 0);
  }

  bool
  RootFile::setIfFastClonable(int remainingEvents, int remainingSubRuns) const {
    if (!fileFormatVersion_.fastCopyPossible()) return false;
    if (!fileIndex_.allEventsInEntryOrder()) return false;
    if (!whichEventsToProcess_.empty()) return false;
    if (eventsToSkip_ != 0) return false;
    if (remainingEvents >= 0 && eventTree_.entries() > remainingEvents) return false;
    if (remainingSubRuns >= 0 && subRunTree_.entries() > remainingSubRuns) return false;
    if (processingMode_ != InputSource::RunsSubRunsAndEvents) return false;
    if (forcedRunOffset_ != 0) return false;
    // Find entry for first event in file
    FileIndex::const_iterator it = fileIndexBegin_;
    while(it != fileIndexEnd_ && it->getEntryType() != FileIndex::kEvent) {
      ++it;
    }
    if (it == fileIndexEnd_) return false;
    if (startAtRun_ > it->run_) return false;
    if (startAtRun_ == it->run_) {
      if (startAtSubRun_ > it->subRun_) return false;
      if (startAtEvent_ > it->event_) return false;
    }
    for (std::vector<SubRunID>::const_iterator it = whichSubRunsToSkip_.begin(),
          itEnd = whichSubRunsToSkip_.end(); it != itEnd; ++it) {
        if (fileIndex_.findSubRunPosition(it->run(), it->subRun(), true) != fileIndexEnd_) {
          // We must skip a subRun in this file.  We will simply assume that
          // it may contain an event, in which case we cannot fast copy.
          return false;
        }
    }
    return true;
  }


  int
  RootFile::setForcedRunOffset(RunNumber_t const& forcedRunNumber) {
    if (fileIndexBegin_ == fileIndexEnd_) return 0;
    int defaultOffset = (fileIndexBegin_->run_ != 0 ? 0 : 1);
    forcedRunOffset_ = (forcedRunNumber != 0U ? forcedRunNumber - fileIndexBegin_->run_ : defaultOffset);
    if (forcedRunOffset_ != 0) {
      fastClonable_ = false;
    }
    return forcedRunOffset_;
  }

  boost::shared_ptr<FileBlock>
  RootFile::createFileBlock() const {
    return boost::shared_ptr<FileBlock>(new FileBlock(fileFormatVersion_,
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

  std::string const&
  RootFile::newBranchToOldBranch(std::string const& newBranch) const {
    std::map<std::string, std::string>::const_iterator it = newBranchToOldBranch_.find(newBranch);
    if (it != newBranchToOldBranch_.end()) {
      return it->second;
    }
    return newBranch;
  }

  FileIndex::EntryType
  RootFile::getEntryType() const {
    if (fileIndexIter_ == fileIndexEnd_) {
      return FileIndex::kEnd;
    }
    return fileIndexIter_->getEntryType();
  }

  // Temporary KLUDGE until we can properly merge runs and subRuns across files
  // This KLUDGE skips duplicate run or subRun entries.
  FileIndex::EntryType
  RootFile::getEntryTypeSkippingDups() {
    if (fileIndexIter_ == fileIndexEnd_) {
      return FileIndex::kEnd;
    }
    if (fileIndexIter_->event_ == 0 && fileIndexIter_ != fileIndexBegin_) {
      if ((fileIndexIter_-1)->run_ == fileIndexIter_->run_ && (fileIndexIter_-1)->subRun_ == fileIndexIter_->subRun_) {
        ++fileIndexIter_;
        return getEntryTypeSkippingDups();
      }
    }
    return fileIndexIter_->getEntryType();
  }

  FileIndex::EntryType
  RootFile::getNextEntryTypeWanted() {
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
    RunNumber_t const& currentRun = fileIndexIter_->run_;
    RunNumber_t correctedCurrentRun = (currentRun ? currentRun : 1U);
    if (specifiedEvents) {
       // We are processing specified events.
      if (correctedCurrentRun > eventListIter_->run()) {
        // The next specified event is in a run not in the file or already passed.  Skip the event
        ++eventListIter_;
        return getNextEntryTypeWanted();
      }
      // Skip any runs before the next specified event.
      if (correctedCurrentRun < eventListIter_->run()) {
        fileIndexIter_ = fileIndex_.findRunPosition(eventListIter_->run(), false);
        return getNextEntryTypeWanted();
      }
    }
    if (entryType == FileIndex::kRun) {
      // Skip any runs before the first run specified, startAtRun_.
      if (correctedCurrentRun < startAtRun_) {
        fileIndexIter_ = fileIndex_.findRunPosition(startAtRun_, false);
        return getNextEntryTypeWanted();
      }
      return FileIndex::kRun;
    } else if (processingMode_ == InputSource::Runs) {
      fileIndexIter_ = fileIndex_.findRunPosition(currentRun + 1, false);
      return getNextEntryTypeWanted();
    }
    SubRunNumber_t const& currentSubRun = fileIndexIter_->subRun_;
    if (specifiedEvents) {
      // We are processing specified events.
      assert (correctedCurrentRun == eventListIter_->run());
      // Get the subRun number of the next specified event.
      FileIndex::const_iterator iter = fileIndex_.findEventPosition(currentRun, 0U, eventListIter_->event(), true);
      if (iter == fileIndexEnd_ || currentSubRun > iter->subRun_) {
        // Event Not Found or already passed. Skip the next specified event;
        ++eventListIter_;
        return getNextEntryTypeWanted();
      }
      // Skip any subRuns before the next specified event.
      if (currentSubRun < iter->subRun_) {
        fileIndexIter_ = fileIndex_.findPosition(eventListIter_->run(), iter->subRun_, 0U);
        return getNextEntryTypeWanted();
      }
    }
    if (entryType == FileIndex::kSubRun) {
      // Skip any subRuns before the first subRun specified, startAtSubRun_.
      assert(correctedCurrentRun >= startAtRun_);
      if (correctedCurrentRun == startAtRun_ && currentSubRun < startAtSubRun_) {
        fileIndexIter_ = fileIndex_.findSubRunOrRunPosition(currentRun, startAtSubRun_);
        return getNextEntryTypeWanted();
      }
      // Skip the subRun if it is in whichSubRunsToSkip_.
      if (binary_search_all(whichSubRunsToSkip_, SubRunID(correctedCurrentRun, currentSubRun))) {
        fileIndexIter_ = fileIndex_.findSubRunOrRunPosition(currentRun, currentSubRun + 1);
        return getNextEntryTypeWanted();
      }
      return FileIndex::kSubRun;
    } else if (processingMode_ == InputSource::RunsAndSubRuns) {
      fileIndexIter_ = fileIndex_.findSubRunOrRunPosition(currentRun, currentSubRun + 1);
      return getNextEntryTypeWanted();
    }
    assert (entryType == FileIndex::kEvent);
    // Skip any events before the first event specified, startAtEvent_.
    assert(correctedCurrentRun >= startAtRun_);
    assert(correctedCurrentRun > startAtRun_ || currentSubRun >= startAtSubRun_);
    if (correctedCurrentRun == startAtRun_ &&
        fileIndexIter_->event_ < startAtEvent_) {
      fileIndexIter_ = fileIndex_.findPosition(currentRun, currentSubRun, startAtEvent_);
      return getNextEntryTypeWanted();
    }
    if (specifiedEvents) {
      // We have specified events to process and we've already positioned the file
      // to execute the run and subRun entry for the current event in the list.
      // Just position to the right event.
      assert (correctedCurrentRun == eventListIter_->run());
      fileIndexIter_ = fileIndex_.findEventPosition(currentRun, currentSubRun,
                                                  eventListIter_->event(),
                                                  false);
      if (fileIndexIter_->event_ != eventListIter_->event()) {
        // Event was not found.
        ++eventListIter_;
        return getNextEntryTypeWanted();
      }
      // Event was found.
      // For the next time around move to the next specified event
      ++eventListIter_;

      if (duplicateChecker_.get() != 0 &&
          duplicateChecker_->isDuplicateAndCheckActive(EventID(fileIndexIter_->run_, fileIndexIter_->event_),
                                                       fileIndexIter_->subRun_,
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
        duplicateChecker_->isDuplicateAndCheckActive(EventID(fileIndexIter_->run_, fileIndexIter_->event_),
                                                     fileIndexIter_->subRun_,
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
          duplicateChecker_->isDuplicateAndCheckActive(EventID(fileIndexIter_->run_, fileIndexIter_->event_),
                                                       fileIndexIter_->subRun_,
                                                       file_)) {
          ++fileIndexIter_;
        }
      }
      return getNextEntryTypeWanted();
    }
    return FileIndex::kEvent;
  }

  void
  RootFile::fillFileIndex() {
    // This function is for backward compatibility only.
    // Newer files store the file index.
    SubRunNumber_t lastSubRun = 0;
    RunNumber_t lastRun = 0;

    // Loop over event entries and fill the index from the event auxiliary branch
    while(eventTree_.next()) {
      fillEventAuxiliary();
      fileIndex_.addEntry(eventAux_.run(),
                          eventAux_.subRun(),
                          eventAux_.event(),
                          eventTree_.entryNumber());
      // If the subRun tree is invalid, use the event tree to add subRun index entries.
      if (!subRunTree_.isValid()) {
        if (lastSubRun != eventAux_.subRun()) {
          lastSubRun = eventAux_.subRun();
          fileIndex_.addEntry(eventAux_.run(), eventAux_.subRun(), 0U, FileIndex::Element::invalidEntry);
        }
      }
      // If the run tree is invalid, use the event tree to add run index entries.
      if (!runTree_.isValid()) {
        if (lastRun != eventAux_.run()) {
          lastRun = eventAux_.run();
          fileIndex_.addEntry(eventAux_.run(), 0U, 0U, FileIndex::Element::invalidEntry);
        }
      }
    }
    eventTree_.setEntryNumber(-1);

    // Loop over subRun entries and fill the index from the subRun auxiliary branch
    if (subRunTree_.isValid()) {
      while (subRunTree_.next()) {
        fillSubRunAuxiliary();
        fileIndex_.addEntry(subRunAux_.run(), subRunAux_.subRun(), 0U, subRunTree_.entryNumber());
      }
      subRunTree_.setEntryNumber(-1);
    }

    // Loop over run entries and fill the index from the run auxiliary branch
    if (runTree_.isValid()) {
      while (runTree_.next()) {
        fillRunAuxiliary();
        fileIndex_.addEntry(runAux_.run(), 0U, 0U, runTree_.entryNumber());
      }
      runTree_.setEntryNumber(-1);
    }
    fileIndex_.sortBy_Run_SubRun_Event();
  }

  void
  RootFile::validateFile() {
    if (!fileFormatVersion_.isValid()) {
      fileFormatVersion_.value_ = 0;
    }
    if (!fid_.isValid()) {
      fid_ = FileID(createGlobalIdentifier());
    }
    if(!eventTree_.isValid()) {
      throw edm::Exception(errors::EventCorruption) <<
         "'Events' tree is corrupted or not present\n" << "in the input file.\n";
    }
    if (fileIndex_.empty()) {
      fillFileIndex();
    }
  }

  void
  RootFile::reportOpened() {
    // Report file opened.
    std::string const label = "source";
    std::string moduleName = "PoolSource";
  }

  void
  RootFile::close(bool reallyClose) {
    if (reallyClose) {
      filePtr_->Close();
    }
  }

  void
  RootFile::fillEventAuxiliary() {
    if (fileFormatVersion_.value_ >= 3) {
      EventAuxiliary *pEvAux = &eventAux_;
      eventTree_.fillAux<EventAuxiliary>(pEvAux);
    } else {
      // for backward compatibility.
      EventAux eventAux;
      EventAux *pEvAux = &eventAux;
      eventTree_.fillAux<EventAux>(pEvAux);
      conversion(eventAux, eventAux_);
    }
    if (eventAux_.subRun_ == 0 && fileFormatVersion_.value_ <= 3) {
      eventAux_.subRun_ = SubRunNumber_t(1);
    } else if (fileFormatVersion_.value_ <= 1) {
      eventAux_.subRun_ = SubRunNumber_t(1);
    }
  }

  void
  RootFile::fillHistory() {
    // We could consider doing delayed reading, but because we have to
    // store this History object in a different tree than the event
    // data tree, this is too hard to do in this first version.

    if (fileFormatVersion_.value_ >= 7) {
      History* pHistory = history_.get();
      TBranch* eventHistoryBranch = eventHistoryTree_->GetBranch(poolNames::eventHistoryBranchName().c_str());
      if (!eventHistoryBranch)
        throw edm::Exception(errors::EventCorruption)
          << "Failed to find history branch in event history tree.\n";
      eventHistoryBranch->SetAddress(&pHistory);
      input::getEntry(eventHistoryTree_, eventTree_.entryNumber());
    } else {
      // for backward compatibility.  If we could figure out how many
      // processes this event has been through, we should fill in
      // history_ with that many default-constructed IDs.
      if (!eventProcessHistoryIDs_.empty()) {
        if (eventProcessHistoryIter_->eventID_ != eventAux_.id()) {
          EventProcessHistoryID target(eventAux_.id(), ProcessHistoryID());
          eventProcessHistoryIter_ = lower_bound_all(eventProcessHistoryIDs_, target);
          assert(eventProcessHistoryIter_->eventID_ == eventAux_.id());
        }
        history_->setProcessHistoryID(eventProcessHistoryIter_->processHistoryID_);
        ++eventProcessHistoryIter_;
      }
    }
    if (fileFormatVersion_.value_ < 11) {
      // old format.  branchListIndexes_ must be filled in from the ProvenanceAdaptor.
      provenanceAdaptor_->branchListIndexes(history_->branchListIndexes());
    }
  }

  void
  RootFile::fillSubRunAuxiliary() {
    if (fileFormatVersion_.value_ >= 3) {
      SubRunAuxiliary *pSubRunAux = &subRunAux_;
      subRunTree_.fillAux<SubRunAuxiliary>(pSubRunAux);
    } else {
      SubRunAux subRunAux;
      SubRunAux *pSubRunAux = &subRunAux;
      subRunTree_.fillAux<SubRunAux>(pSubRunAux);
      conversion(subRunAux, subRunAux_);
    }
    if (subRunAux_.subRun() == 0 && fileFormatVersion_.value_ <= 3) {
      subRunAux_.id_ = SubRunID(subRunAux_.run(), SubRunNumber_t(1));
    }
  }

  void
  RootFile::fillRunAuxiliary() {
    if (fileFormatVersion_.value_ >= 3) {
      RunAuxiliary *pRunAux = &runAux_;
      runTree_.fillAux<RunAuxiliary>(pRunAux);
    } else {
      RunAux runAux;
      RunAux *pRunAux = &runAux;
      runTree_.fillAux<RunAux>(pRunAux);
      conversion(runAux, runAux_);
    }
  }

  int
  RootFile::skipEvents(int offset) {
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
  std::auto_ptr<EventPrincipal>
  RootFile::readEvent(boost::shared_ptr<ProductRegistry const> pReg) {
    assert(fileIndexIter_ != fileIndexEnd_);
    assert(fileIndexIter_->getEntryType() == FileIndex::kEvent);
    RunNumber_t currentRun = (fileIndexIter_->run_ ? fileIndexIter_->run_ : 1U);
    assert(currentRun >= startAtRun_);
    assert(currentRun > startAtRun_ || fileIndexIter_->subRun_ >= startAtSubRun_);
    assert(currentRun > startAtRun_ || fileIndexIter_->subRun_ > startAtSubRun_ ||
         fileIndexIter_->event_ >= startAtEvent_);
    // Set the entry in the tree, and read the event at that entry.
    eventTree_.setEntryNumber(fileIndexIter_->entry_);
    std::auto_ptr<EventPrincipal> ep = readCurrentEvent(pReg);

    assert(ep.get() != 0);
    assert(eventAux_.run() == fileIndexIter_->run_ + forcedRunOffset_);
    assert(eventAux_.subRun() == fileIndexIter_->subRun_);

    // report event read from file
    ++fileIndexIter_;
    return ep;
  }

  // Reads event at the current entry in the tree.
  // Note: This function neither uses nor sets fileIndexIter_.
  std::auto_ptr<EventPrincipal>
  RootFile::readCurrentEvent(boost::shared_ptr<ProductRegistry const> pReg) {
    if (!eventTree_.current()) {
      return std::auto_ptr<EventPrincipal>(0);
    }
    fillEventAuxiliary();
    fillHistory();
    overrideRunNumber(eventAux_.id_, eventAux_.isRealData());

    boost::shared_ptr<BranchMapper> mapper = (fileFormatVersion().value_ >= 11 ?
        makeBranchMapper<ProductProvenance>(eventTree_, InEvent) :
        makeBranchMapper<EventEntryInfo>(eventTree_, InEvent));

    // We're not done ... so prepare the EventPrincipal
    std::auto_ptr<EventPrincipal> thisEvent(new EventPrincipal(
                eventAux_,
                pReg,
                processConfiguration_,
                history_,
                mapper,
                eventTree_.makeDelayedReader(fileFormatVersion_.value_ < 11)));

    // Create a group in the event for each product
    eventTree_.fillGroups(*thisEvent);
    if (fileFormatVersion().value_ < 11 && fileFormatVersion().value_ >= 8) {
      thisEvent->readProvenanceImmediate();
    }
    return thisEvent;
  }

  void
  RootFile::setAtEventEntry(FileIndex::EntryNumber_t entry) {
    eventTree_.setEntryNumber(entry);
  }

  boost::shared_ptr<RunPrincipal>
  RootFile::readRun(boost::shared_ptr<ProductRegistry const> pReg) {
    assert(fileIndexIter_ != fileIndexEnd_);
    assert(fileIndexIter_->getEntryType() == FileIndex::kRun);
    RunNumber_t currentRun = (fileIndexIter_->run_ ? fileIndexIter_->run_ : 1U);
    assert(currentRun >= startAtRun_);
    // Begin code for backward compatibility before the exixtence of run trees.
    if (!runTree_.isValid()) {
      // prior to the support of run trees.
      // RunAuxiliary did not contain a valid timestamp.  Take it from the next event.
      if (eventTree_.next()) {
        fillEventAuxiliary();
        // back up, so event will not be skipped.
        eventTree_.previous();
      }
      RunID run = RunID(fileIndexIter_->run_);
      overrideRunNumber(run);
      ++fileIndexIter_;
      RunAuxiliary runAux(run.run(), eventAux_.time(), Timestamp::invalidTimestamp());
      return boost::shared_ptr<RunPrincipal>(
          new RunPrincipal(runAux,
          pReg,
          processConfiguration_));
    }
    // End code for backward compatibility before the exixtence of run trees.
    runTree_.setEntryNumber(fileIndexIter_->entry_);
    fillRunAuxiliary();
    assert(runAux_.run() == fileIndexIter_->run_);
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
    boost::shared_ptr<RunPrincipal> thisRun(
        new RunPrincipal(runAux_,
                         pReg,
                         processConfiguration_,
                         fileFormatVersion().value_ <= 10 && fileFormatVersion().value_ >= 8 ?
                         makeBranchMapper<RunSubRunEntryInfo>(runTree_, InRun) :
                         makeBranchMapper<ProductProvenance>(runTree_, InRun),
                         runTree_.makeDelayedReader()));
    // Create a group in the run for each product
    runTree_.fillGroups(*thisRun);
    // Read in all the products now.
    thisRun->readImmediate();
    ++fileIndexIter_;
    return thisRun;
  }

  boost::shared_ptr<SubRunPrincipal>
  RootFile::readSubRun(boost::shared_ptr<ProductRegistry const> pReg, boost::shared_ptr<RunPrincipal> rp) {
    assert(fileIndexIter_ != fileIndexEnd_);
    assert(fileIndexIter_->getEntryType() == FileIndex::kSubRun);
    RunNumber_t currentRun = (fileIndexIter_->run_ ? fileIndexIter_->run_ : 1U);
    assert(currentRun >= startAtRun_);
    assert(currentRun > startAtRun_ || fileIndexIter_->subRun_ >= startAtSubRun_);
    // Begin code for backward compatibility before the existence of subRun trees.
    if (!subRunTree_.isValid()) {
      if (eventTree_.next()) {
        fillEventAuxiliary();
        // back up, so event will not be skipped.
        eventTree_.previous();
      }

      SubRunID subRun = SubRunID(fileIndexIter_->run_, fileIndexIter_->subRun_);
      overrideRunNumber(subRun);
      ++fileIndexIter_;
      SubRunAuxiliary subRunAux(rp->run(), subRun.subRun(), eventAux_.time_, Timestamp::invalidTimestamp());
      return boost::shared_ptr<SubRunPrincipal>(
        new SubRunPrincipal(subRunAux,
                                     pReg,
                                     processConfiguration_));
    }
    // End code for backward compatibility before the exixtence of subRun trees.
    subRunTree_.setEntryNumber(fileIndexIter_->entry_);
    fillSubRunAuxiliary();
    assert(subRunAux_.run() == fileIndexIter_->run_);
    assert(subRunAux_.subRun() == fileIndexIter_->subRun_);
    overrideRunNumber(subRunAux_.id_);
    assert(subRunAux_.run() == rp->run());

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
    boost::shared_ptr<SubRunPrincipal> thisSubRun(
        new SubRunPrincipal(subRunAux_,
                                     pReg, processConfiguration_,
                                     fileFormatVersion().value_ <= 10 && fileFormatVersion().value_ >= 8 ?
                                     makeBranchMapper<RunSubRunEntryInfo>(subRunTree_, InSubRun) :
                                     makeBranchMapper<ProductProvenance>(subRunTree_, InSubRun),
                                     subRunTree_.makeDelayedReader()));
    // Create a group in the subRun for each product
    subRunTree_.fillGroups(*thisSubRun);
    // Read in all the products now.
    thisSubRun->readImmediate();
    ++fileIndexIter_;
    return thisSubRun;
  }

  bool
  RootFile::setEntryAtEvent(RunNumber_t run, SubRunNumber_t subRun, EventNumber_t event, bool exact) {
    fileIndexIter_ = fileIndex_.findEventPosition(run, subRun, event, exact);
    if (fileIndexIter_ == fileIndexEnd_) return false;
    eventTree_.setEntryNumber(fileIndexIter_->entry_);
    return true;
  }

  bool
  RootFile::setEntryAtSubRun(SubRunID const& subRun) {
    fileIndexIter_ = fileIndex_.findSubRunPosition(subRun.run(), subRun.subRun(), true);
    if (fileIndexIter_ == fileIndexEnd_) return false;
    subRunTree_.setEntryNumber(fileIndexIter_->entry_);
    return true;
  }

  bool
  RootFile::setEntryAtRun(RunID const& run) {
    fileIndexIter_ = fileIndex_.findRunPosition(run.run(), true);
    if (fileIndexIter_ == fileIndexEnd_) return false;
    runTree_.setEntryNumber(fileIndexIter_->entry_);
    return true;
  }

  void
  RootFile::overrideRunNumber(RunID & id) {
    if (forcedRunOffset_ != 0) {
      id = RunID(id.run() + forcedRunOffset_);
    }
    if (id < RunID::firstValidRun()) id = RunID::firstValidRun();
  }

  void
  RootFile::overrideRunNumber(SubRunID & id) {
    if (forcedRunOffset_ != 0) {
      id = SubRunID(id.run() + forcedRunOffset_, id.subRun());
    }
    if (RunID(id.run()) < RunID::firstValidRun()) id = SubRunID(RunID::firstValidRun().run(), id.subRun());
  }

  void
  RootFile::overrideRunNumber(EventID & id, bool isRealData) {
    if (forcedRunOffset_ != 0) {
      if (isRealData) {
        throw edm::Exception(errors::Configuration,"RootFile::RootFile()")
          << "The 'setRunNumber' parameter of PoolSource cannot be used with real data.\n";
      }
      id = EventID(id.run() + forcedRunOffset_, id.event());
    }
    if (RunID(id.run()) < RunID::firstValidRun()) id = EventID(RunID::firstValidRun().run(), id.event());
  }


  void
  RootFile::readEventHistoryTree() {
    // Read in the event history tree, if we have one...
    if (fileFormatVersion_.value_ < 7) return;
    eventHistoryTree_ = dynamic_cast<TTree*>(filePtr_->Get(poolNames::eventHistoryTreeName().c_str()));

    if (!eventHistoryTree_)
      throw edm::Exception(errors::EventCorruption)
        << "Failed to find the event history tree.\n";
  }

  void
  RootFile::initializeDuplicateChecker() {
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
  RootFile::dropOnInput (GroupSelectorRules const& rules, bool dropDescendants, bool dropMergeable) {
    // This is the selector for drop on input.
    GroupSelector groupSelector;
    groupSelector.initialize(rules, productRegistry()->allBranchDescriptions());

    ProductRegistry::ProductList& prodList = const_cast<ProductRegistry::ProductList&>(productRegistry()->productList());
    // Do drop on input. On the first pass, just fill in a set of branches to be dropped.
    std::set<BranchID> branchesToDrop;
    for (ProductRegistry::ProductList::const_iterator it = prodList.begin(), itEnd = prodList.end();
        it != itEnd; ++it) {
      BranchDescription const& prod = it->second;
      if(!groupSelector.selected(prod)) {
        if (dropDescendants) {
          branchChildren_->appendToDescendants(prod.branchID(), branchesToDrop);
        } else {
          branchesToDrop.insert(prod.branchID());
        }
      }
    }

    // On this pass, actually drop the branches.
    std::set<BranchID>::const_iterator branchesToDropEnd = branchesToDrop.end();
    for (ProductRegistry::ProductList::iterator it = prodList.begin(), itEnd = prodList.end(); it != itEnd;) {
      BranchDescription const& prod = it->second;
      bool drop = branchesToDrop.find(prod.branchID()) != branchesToDropEnd;
      if(drop) {
        if (groupSelector.selected(prod)) {
          mf::LogWarning("RootFile")
            << "Branch '" << prod.branchName() << "' is being dropped from the input\n"
            << "of file '" << file_ << "' because it is dependent on a branch\n"
            << "that was explicitly dropped.\n";
        }
        treePointers_[prod.branchType()]->dropBranch(newBranchToOldBranch(prod.branchName()));
        ProductRegistry::ProductList::iterator icopy = it;
        ++it;
        prodList.erase(icopy);
      } else {
        ++it;
      }
    }

    // Drop on input mergeable run and subRun products, this needs to be invoked for
    // secondary file input
    if (dropMergeable) {
      for (ProductRegistry::ProductList::iterator it = prodList.begin(), itEnd = prodList.end(); it != itEnd;) {
        BranchDescription const& prod = it->second;
        if (prod.branchType() != InEvent) {
          TClass *cp = gROOT->GetClass(prod.wrappedName().c_str());
          boost::shared_ptr<EDProduct> dummy(static_cast<EDProduct *>(cp->New()));
          if (dummy->isMergeable()) {
            treePointers_[prod.branchType()]->dropBranch(newBranchToOldBranch(prod.branchName()));
            ProductRegistry::ProductList::iterator icopy = it;
            ++it;
            prodList.erase(icopy);
          } else {
            ++it;
          }
        }
        else ++it;
      }
    }
  }

  // backward compatibility
  boost::shared_ptr<BranchMapper>
  RootFile:: makeBranchMapperInOldRelease(RootTree & rootTree, BranchType const& type) const {
    if (fileFormatVersion_.value_ >= 7) {
      rootTree.fillStatus();
    } else {
       mf::LogWarning("RootFile")
         << "Backward compatibility not fully supported for reading files"
            " written in CMSSW_1_8_4 or prior releases in releaseCMSSW_3_0_0.\n";
    }
    if (type == InEvent) {
      boost::shared_ptr<BranchMapperWithReader<EventEntryInfo> > mapper(new BranchMapperWithReader<EventEntryInfo>(0, 0));
      mapper->setDelayedRead(false);
      for(ProductRegistry::ProductList::const_iterator it = productRegistry_->productList().begin(),
          itEnd = productRegistry_->productList().end(); it != itEnd; ++it) {
        if (type == it->second.branchType() && !it->second.transient()) {
          if (fileFormatVersion_.value_ >= 7) {
            input::BranchMap::const_iterator ix = rootTree.branches().find(it->first);
            input::BranchInfo const& ib = ix->second;
            TBranch *br = ib.provenanceBranch_;
            std::auto_ptr<EntryDescriptionID> pb(new EntryDescriptionID);
            EntryDescriptionID* ppb = pb.get();
            br->SetAddress(&ppb);
            input::getEntry(br, rootTree.entryNumber());
            std::vector<ProductStatus>::size_type index = it->second.oldProductID().productIndex() - 1;
            EventEntryInfo entry(it->second.branchID(), rootTree.productStatuses()[index], it->second.oldProductID(), *pb);
            mapper->insert(entry.makeProductProvenance());
          } else {
            TBranch *br = rootTree.branches().find(it->first)->second.provenanceBranch_;
            std::auto_ptr<BranchEntryDescription> pb(new BranchEntryDescription);
            BranchEntryDescription* ppb = pb.get();
            br->SetAddress(&ppb);
            input::getEntry(br, rootTree.entryNumber());
            std::auto_ptr<EntryDescription> entryDesc = pb->convertToEntryDescription();
            ProductStatus status = (ppb->creatorStatus() == BranchEntryDescription::Success ? productstatus::present() : productstatus::neverCreated());
            EventEntryInfo entry(it->second.branchID(), status, it->second.oldProductID());
            mapper->insert(entry.makeProductProvenance());
          }
          mapper->insertIntoMap(it->second.oldProductID(), it->second.branchID());
        }
      }
      return mapper;
    } else {
      boost::shared_ptr<BranchMapperWithReader<ProductProvenance> > mapper(new BranchMapperWithReader<ProductProvenance>(0, 0));
      mapper->setDelayedRead(false);
      for(ProductRegistry::ProductList::const_iterator it = productRegistry_->productList().begin(),
          itEnd = productRegistry_->productList().end(); it != itEnd; ++it) {
        if (type == it->second.branchType() && !it->second.transient()) {
          if (fileFormatVersion_.value_ >= 7) {
            input::BranchMap::const_iterator ix = rootTree.branches().find(it->first);
            input::BranchInfo const& ib = ix->second;
            TBranch *br = ib.provenanceBranch_;
            input::getEntry(br, rootTree.entryNumber());
            std::vector<ProductStatus>::size_type index = it->second.oldProductID().productIndex() - 1;
            ProductProvenance entry(it->second.branchID(), rootTree.productStatuses()[index]);
            mapper->insert(entry);
          } else {
            TBranch *br = rootTree.branches().find(it->first)->second.provenanceBranch_;
            std::auto_ptr<BranchEntryDescription> pb(new BranchEntryDescription);
            BranchEntryDescription* ppb = pb.get();
            br->SetAddress(&ppb);
            input::getEntry(br, rootTree.entryNumber());
            std::auto_ptr<EntryDescription> entryDesc = pb->convertToEntryDescription();
            ProductStatus status = (ppb->creatorStatus() == BranchEntryDescription::Success ? productstatus::present() : productstatus::neverCreated());
            ProductProvenance entry(it->second.branchID(), status);
            mapper->insert(entry);
          }
        }
      }
      return mapper;
    }
    return boost::shared_ptr<BranchMapper>();
  }
  // end backward compatibility

}  // namespace edm
