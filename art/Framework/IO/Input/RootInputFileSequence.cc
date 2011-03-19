// ======================================================================
//
// RootInputFileSequence
//
// ======================================================================

#include "art/Framework/IO/Input/RootInputFileSequence.h"

#ifdef USE_RANDOM
// #include "FWCore/Utilities/interface/RandomNumberGenerator.h"
#include "CLHEP/Random/RandFlat.h"
#endif
// #include "Utilities/StorageFactory/interface/StorageFactory.h"

#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/IO/Catalog/FileCatalog.h"
#include "art/Framework/IO/Input/DuplicateChecker.h"
#include "art/Framework/IO/Input/RootInput.h"
#include "art/Framework/IO/Input/RootInputFile.h"
#include "art/Framework/IO/Input/RootTree.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "TFile.h"
#include <ctime>

using namespace cet;
using namespace std;

namespace art {

  RootInputFileSequence::RootInputFileSequence( fhicl::ParameterSet const& pset,
                                                RootInput const& input,
                                                InputFileCatalog const& catalog,
                                                bool primarySequence) :
    input_(input),
    catalog_(catalog),
    firstFile_(true),
    fileIterBegin_(fileCatalogItems().begin()),
    fileIterEnd_(fileCatalogItems().end()),
    fileIter_(fileIterEnd_),
    rootFile_(),
    matchMode_(BranchDescription::Permissive),
    flatDistribution_(0),
    fileIndexes_(fileCatalogItems().size()),
    eventsRemainingInFile_(0),
    origEventID_(),
    eventsToSkip_(pset.get<EventNumber_t>("skipEvents", (EventNumber_t)0)),
    whichSubRunsToSkip_(),
    eventsToProcess_(),
    noEventSort_(pset.get<bool>("noEventSort", false)),
    skipBadFiles_(pset.get<bool>("skipBadFiles", false)),
    treeCacheSize_(pset.get<unsigned int>("cacheSize", 0U)),
    treeMaxVirtualSize_(pset.get<int>("treeMaxVirtualSize", -1)),
    forcedRunOffset_(0),
    setRun_(pset.get<unsigned int>("setRunNumber", 0U)),
    groupSelectorRules_(pset, "inputCommands", "InputSource"),
    primarySequence_(primarySequence),
    randomAccess_(false),
    duplicateChecker_(),
    dropDescendants_(pset.get<bool>("dropDescendantsOfDroppedBranches", true)) {

//     startAtRun_(pset.get<unsigned int>("firstRun", 1U)),
//     startAtSubRun_(pset.get<unsigned int>("firstSubRun", 1U)),
//     startAtEvent_(pset.get<unsigned int>("firstEvent", 1U)),

    RunNumber_t firstRun;
    bool haveFirstRun = pset.get_if_present("firstRun", firstRun);
    SubRunNumber_t firstSubRun;
    bool haveFirstSubRun = pset.get_if_present("firstSubRun", firstSubRun);
    EventNumber_t firstEvent;
    bool haveFirstEvent = pset.get_if_present("firstEvent", firstEvent);
    RunID firstRunID = haveFirstRun?RunID(firstRun):RunID::firstRun();
    SubRunID firstSubRunID = haveFirstSubRun?SubRunID(firstRunID.run(), firstSubRun):
       SubRunID::firstSubRun(firstRunID);
    origEventID_ = haveFirstEvent?EventID(firstSubRunID.run(),
                                          firstSubRunID.subRun(),
                                          firstEvent):
       EventID::firstEvent(firstSubRunID);

    if (!primarySequence_) noEventSort_ = false;
    if (noEventSort_ && (haveFirstEvent || !eventsToProcess_.empty())) {
      throw art::Exception(errors::Configuration)
        << "Illegal configuration options passed to RootInput\n"
        << "You cannot request \"noEventSort\" and also set \"firstEvent\"\n"
        << "or \"eventsToProcess\".\n";
    }

    if (primarySequence_ && primary()) duplicateChecker_.reset(new DuplicateChecker(pset));


    sort_all(eventsToProcess_);
    string matchMode = pset.get<string>("fileMatchMode", string("permissive"));
    if (matchMode == string("strict")) matchMode_ = BranchDescription::Strict;
    if (primary()) {
      for(fileIter_ = fileIterBegin_; fileIter_ != fileIterEnd_; ++fileIter_) {
        initFile(skipBadFiles_);
        if (rootFile_) break;
      }
      if (rootFile_) {
        forcedRunOffset_ = rootFile_->setForcedRunOffset(setRun_);
        if (forcedRunOffset_ < 0) {
          throw art::Exception(errors::Configuration)
            << "The value of the 'setRunNumber' parameter must not be\n"
            << "less than the first run number in the first input file.\n"
            << "'setRunNumber' was " << setRun_ <<", while the first run was "
            << setRun_ - forcedRunOffset_ << ".\n";
        }
        productRegistryUpdate().updateFromInput(rootFile_->productRegistry()->productList());
        BranchIDListHelper::updateFromInput(rootFile_->branchIDLists(), fileIter_->fileName());
      }
    } else {
#ifdef USE_RANDOM
      ServiceHandle<RandomNumberGenerator> rng;
      if (!rng.isAvailable()) {
        throw art::Exception(errors::Configuration)
          << "A secondary input source requires the RandomNumberGenerator service\n"
          << "which is not present in the configuration file.  You must add the service\n"
          << "in the configuration file or remove the modules that require it.";
      }
      CLHEP::HepRandomEngine& engine = rng->getEngine();
      flatDistribution_ = new CLHEP::RandFlat(engine);
#endif
    }
  }

  vector<FileCatalogItem> const&
  RootInputFileSequence::fileCatalogItems() const {
    return catalog_.fileCatalogItems();
  }

  void
  RootInputFileSequence::endJob() {
    closeFile_();
  }

  boost::shared_ptr<FileBlock>
  RootInputFileSequence::readFile_() {
    if (firstFile_) {
      // The first input file has already been opened, or a rewind has occurred.
      firstFile_ = false;
      if (!rootFile_) {
        initFile(skipBadFiles_);
      }
    } else {
      if (!nextFile()) {
        assert(0);
      }
    }
    if (!rootFile_) {
      return boost::shared_ptr<FileBlock>(new FileBlock);
    }
    return rootFile_->createFileBlock();
  }

  void RootInputFileSequence::closeFile_() {
    if (rootFile_) {
    // Account for events skipped in the file.
      eventsToSkip_ = rootFile_->eventsToSkip();
      {
        rootFile_->close(primary());
      }
      logFileAction("  Closed file ", rootFile_->file());
      rootFile_.reset();
      if (duplicateChecker_.get() != 0) duplicateChecker_->inputFileClosed();
    }
  }

  void RootInputFileSequence::initFile(bool skipBadFiles) {
    // close the currently open file, any, and delete the RootInputFile object.
    closeFile_();
    boost::shared_ptr<TFile> filePtr;
    try {
      logFileAction("  Initiating request to open file ", fileIter_->fileName());
      filePtr.reset(TFile::Open(fileIter_->fileName().c_str()));
    }
    catch (cet::exception e) {
      if (!skipBadFiles) {
        throw art::Exception(art::errors::FileOpenError) << e.explain_self() << "\n" <<
           "RootInputFileSequence::initFile(): Input file " << fileIter_->fileName() << " was not found or could not be opened.\n";
      }
    }
    if (filePtr && !filePtr->IsZombie()) {
      logFileAction("  Successfully opened file ", fileIter_->fileName());
      rootFile_ = RootInputFileSharedPtr(new RootInputFile(fileIter_->fileName(), catalog_.url(),
          processConfiguration(), fileIter_->logicalFileName(), filePtr,
          origEventID_, eventsToSkip_, whichSubRunsToSkip_,
          remainingEvents(), remainingSubRuns(), treeCacheSize_, treeMaxVirtualSize_,
          input_.processingMode(),
          forcedRunOffset_, eventsToProcess_, noEventSort_,
          groupSelectorRules_, !primarySequence_, duplicateChecker_, dropDescendants_));
      fileIndexes_[fileIter_ - fileIterBegin_] = rootFile_->fileIndexSharedPtr();
    } else {
      if (!skipBadFiles) {
        throw art::Exception(art::errors::FileOpenError) <<
           "RootInputFileSequence::initFile(): Input file " << fileIter_->fileName() << " was not found or could not be opened.\n";
      }
      mf::LogWarning("")
        << "Input file: " << fileIter_->fileName()
        << " was not found or could not be opened, and will be skipped.\n";
    }
  }

  ProductRegistry const&
  RootInputFileSequence::fileProductRegistry() const {
    return *rootFile_->productRegistry();
  }

  bool RootInputFileSequence::nextFile() {
    if(fileIter_ != fileIterEnd_) ++fileIter_;
    if(fileIter_ == fileIterEnd_) {
      if (primarySequence_) {
        return false;
      } else {
        fileIter_ = fileIterBegin_;
      }
    }

    initFile(skipBadFiles_);

    if (primarySequence_ && rootFile_) {
      // make sure the new product registry is compatible with the main one
      string mergeInfo = productRegistryUpdate().merge(*rootFile_->productRegistry(),
                                                            fileIter_->fileName(),
                                                            matchMode_);
      if (!mergeInfo.empty()) {
        throw art::Exception(errors::MismatchedInputFiles,"RootInputFileSequence::nextFile()") << mergeInfo;
      }
      BranchIDListHelper::updateFromInput(rootFile_->branchIDLists(), fileIter_->fileName());
    }
    return true;
  }

  bool RootInputFileSequence::previousFile() {
    if(fileIter_ == fileIterBegin_) {
      if (primarySequence_) {
        return false;
      } else {
        fileIter_ = fileIterEnd_;
      }
    }
    --fileIter_;

    initFile(false);

    if (primarySequence_ && rootFile_) {
      // make sure the new product registry is compatible to the main one
      string mergeInfo = productRegistryUpdate().merge(*rootFile_->productRegistry(),
                                                            fileIter_->fileName(),
                                                            matchMode_);
      if (!mergeInfo.empty()) {
        throw art::Exception(errors::MismatchedInputFiles,"RootInputFileSequence::previousEvent()") << mergeInfo;
      }
      BranchIDListHelper::updateFromInput(rootFile_->branchIDLists(), fileIter_->fileName());
    }
    if (rootFile_) rootFile_->setToLastEntry();
    return true;
  }

  RootInputFileSequence::~RootInputFileSequence() {
  }

  boost::shared_ptr<RunPrincipal>
  RootInputFileSequence::readRun_() {
    return rootFile_->readRun(primarySequence_ ? productRegistry() : rootFile_->productRegistry());
  }

  boost::shared_ptr<SubRunPrincipal>
  RootInputFileSequence::readSubRun_() {
    return rootFile_->readSubRun(primarySequence_ ? productRegistry() : rootFile_->productRegistry(), runPrincipal());
  }

  // readEvent_() is responsible for creating, and setting up, the
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
  RootInputFileSequence::readEvent_() {
    return rootFile_->readEvent(primarySequence_ ? productRegistry() : rootFile_->productRegistry());
  }

  auto_ptr<EventPrincipal>
  RootInputFileSequence::readCurrentEvent() {
    return rootFile_->readCurrentEvent(primarySequence_ ?
                                       productRegistry() :
                                       rootFile_->productRegistry());
  }

  auto_ptr<EventPrincipal>
  RootInputFileSequence::readIt(EventID const& id, bool exact) {
    randomAccess_ = true;
    // Attempt to find event in currently open input file.
    bool found = rootFile_->setEntryAtEvent(id, exact);
    if (!found) {
      // If only one input file, give up now, to save time.
      if (fileIndexes_.size() == 1) {
        return auto_ptr<EventPrincipal>(0);
      }
      // Look for event in files previously opened without reopening unnecessary files.
      typedef vector<boost::shared_ptr<FileIndex> >::const_iterator Iter;
      for (Iter it = fileIndexes_.begin(), itEnd = fileIndexes_.end(); it != itEnd; ++it) {
        if (*it && (*it)->containsEvent(id, exact)) {
          // We found it. Close the currently open file, and open the correct one.
          fileIter_ = fileIterBegin_ + (it - fileIndexes_.begin());
          initFile(false);
          // Now get the event from the correct file.
          found = rootFile_->setEntryAtEvent(id, exact);
          assert (found);
          auto_ptr<EventPrincipal> ep = readCurrentEvent();
          skip(1);
          return ep;
        }
      }
      // Look for event in files not yet opened.
      for (Iter it = fileIndexes_.begin(), itEnd = fileIndexes_.end(); it != itEnd; ++it) {
        if (!*it) {
          fileIter_ = fileIterBegin_ + (it - fileIndexes_.begin());
          initFile(false);
          found = rootFile_->setEntryAtEvent(id, exact);
          if (found) {
            auto_ptr<EventPrincipal> ep = readCurrentEvent();
            skip(1);
            return ep;
          }
        }
      }
      // Not found
      return auto_ptr<EventPrincipal>(0);
    }
    auto_ptr<EventPrincipal> eptr = readCurrentEvent();
    skip(1);
    return eptr;
  }

  boost::shared_ptr<SubRunPrincipal>
  RootInputFileSequence::readIt(SubRunID const& id) {

    // Attempt to find subRun in currently open input file.
    bool found = rootFile_->setEntryAtSubRun(id);
    if (found) {
      return readSubRun_();
    }

    if (fileIndexes_.size() > 1) {
      // Look for subRun in files previously opened without reopening unnecessary files.
      typedef vector<boost::shared_ptr<FileIndex> >::const_iterator Iter;
      for (Iter it = fileIndexes_.begin(), itEnd = fileIndexes_.end(); it != itEnd; ++it) {
        if (*it && (*it)->containsSubRun(id, true)) {
          // We found it. Close the currently open file, and open the correct one.
          fileIter_ = fileIterBegin_ + (it - fileIndexes_.begin());
          initFile(false);
          // Now get the subRun from the correct file.
          found = rootFile_->setEntryAtSubRun(id);
          assert (found);
          return readSubRun_();
        }
      }
      // Look for subRun in files not yet opened.
      for (Iter it = fileIndexes_.begin(), itEnd = fileIndexes_.end(); it != itEnd; ++it) {
        if (!*it) {
          fileIter_ = fileIterBegin_ + (it - fileIndexes_.begin());
          initFile(false);
          found = rootFile_->setEntryAtSubRun(id);
          if (found) {
            return readSubRun_();
          }
        }
      }
    }
    return boost::shared_ptr<SubRunPrincipal>();
  }

  boost::shared_ptr<RunPrincipal>
  RootInputFileSequence::readIt(RunID const& id) {

    // Attempt to find run in currently open input file.
    bool found = rootFile_->setEntryAtRun(id);
    if (found) {
      return readRun_();
    }
    if (fileIndexes_.size() > 1) {
      // Look for run in files previously opened without reopening unnecessary files.
      typedef vector<boost::shared_ptr<FileIndex> >::const_iterator Iter;
      for (Iter it = fileIndexes_.begin(), itEnd = fileIndexes_.end(); it != itEnd; ++it) {
        if (*it && (*it)->containsRun(id, true)) {
          // We found it. Close the currently open file, and open the correct one.
          fileIter_ = fileIterBegin_ + (it - fileIndexes_.begin());
          initFile(false);
          // Now get the event from the correct file.
          found = rootFile_->setEntryAtRun(id);
          assert (found);
          return readRun_();
        }
      }
      // Look for run in files not yet opened.
      for (Iter it = fileIndexes_.begin(), itEnd = fileIndexes_.end(); it != itEnd; ++it) {
        if (!*it) {
          fileIter_ = fileIterBegin_ + (it - fileIndexes_.begin());
          initFile(false);
          found = rootFile_->setEntryAtRun(id);
          if (found) {
            return readRun_();
          }
        }
      }
    }
    return boost::shared_ptr<RunPrincipal>();
  }

  input::ItemType
  RootInputFileSequence::getNextItemType() {
    if (fileIter_ == fileIterEnd_) {
      return input::IsStop;
    }
    if (firstFile_) {
      return input::IsFile;
    }
    if (rootFile_) {
      if (randomAccess_) {
        skip(0);
        if (fileIter_== fileIterEnd_) {
          return input::IsStop;
        }
      }
      FileIndex::EntryType entryType = rootFile_->getNextEntryTypeWanted();
      if (entryType == FileIndex::kEvent) {
        return input::IsEvent;
      } else if (entryType == FileIndex::kSubRun) {
        return input::IsSubRun;
      } else if (entryType == FileIndex::kRun) {
        return input::IsRun;
      }
      assert(entryType == FileIndex::kEnd);
    }
    if (fileIter_ + 1 == fileIterEnd_) {
      return input::IsStop;
    }
    return input::IsFile;
  }

  // Rewind to before the first event that was read.
  void
  RootInputFileSequence::rewind_() {
    randomAccess_ = false;
    firstFile_ = true;
    fileIter_ = fileIterBegin_;
    if (duplicateChecker_.get() != 0) duplicateChecker_->rewind();
  }

  // Rewind to the beginning of the current file
  void
  RootInputFileSequence::rewindFile() {
    rootFile_->rewind();
  }

  // Advance "offset" events.  Offset can be positive or negative (or zero).
  void
  RootInputFileSequence::skip(int offset) {
    randomAccess_ = true;
    while (offset != 0) {
      offset = rootFile_->skipEvents(offset);
      if (offset > 0 && !nextFile()) return;
      if (offset < 0 && !previousFile()) return;
    }
    rootFile_->skipEvents(0);
  }

  bool const
  RootInputFileSequence::primary() const {
    return input_.primary();
  }

  boost::shared_ptr<RunPrincipal>
  RootInputFileSequence::runPrincipal() const {
    return input_.runPrincipal();
  }

  ProcessConfiguration const&
  RootInputFileSequence::processConfiguration() const {
    return input_.processConfiguration();
  }

  int
  RootInputFileSequence::remainingEvents() const {
    return input_.remainingEvents();
  }

  int
  RootInputFileSequence::remainingSubRuns() const {
    return input_.remainingSubRuns();
  }

  ProductRegistry &
  RootInputFileSequence::productRegistryUpdate() const{
    return input_.productRegistryUpdate();
  }

  cet::exempt_ptr<ProductRegistry const>
  RootInputFileSequence::productRegistry() const{
    return input_.productRegistry();
  }

  void RootInputFileSequence::logFileAction(const char* msg, string const& file) {
    if (primarySequence_) {
      time_t t = time(0);
      char ts[] = "dd-Mon-yyyy hh:mm:ss TZN     ";
      strftime( ts, strlen(ts)+1, "%d-%b-%Y %H:%M:%S %Z", localtime(&t) );
      mf::LogAbsolute("fileAction")
        << ts << msg << file;
      mf::FlushMessageLog();
    }
  }

}  // art

// ======================================================================
