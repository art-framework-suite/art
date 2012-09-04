// ======================================================================
//
// RootInputFileSequence
//
// ======================================================================

#include "art/Framework/IO/Root/RootInputFileSequence.h"

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Framework/IO/Catalog/FileCatalog.h"
#include "art/Framework/IO/Catalog/InputFileCatalog.h"
#include "art/Framework/IO/Root/DuplicateChecker.h"
#include "art/Framework/IO/Root/RootInputFile.h"
#include "art/Framework/IO/Root/RootTree.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "TFile.h"
#include <ctime>

using namespace cet;
using namespace std;

namespace art {

  RootInputFileSequence::RootInputFileSequence( fhicl::ParameterSet const& pset,
                                                InputFileCatalog & catalog,
                                                bool primarySequence,
                                                FastCloningInfoProvider const &fcip,
                                                InputSource::ProcessingMode pMode,
                                                MasterProductRegistry &mpr,
                                                ProcessConfiguration const &processConfig) :
    //    input_(input),
    catalog_(catalog),
    firstFile_(true),
    rootFile_(),
    matchMode_(BranchDescription::Permissive),
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
    duplicateChecker_(),
    dropDescendants_(pset.get<bool>("dropDescendantsOfDroppedBranches", true)),
    fastCloningInfo_(fcip),
    processingMode_(pMode),
    processConfiguration_(processConfig) {

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

    while(catalog_.getNextFile())
    {
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
      mpr.updateFromInput(rootFile_->productList());
      BranchIDListHelper::updateFromInput(rootFile_->branchIDLists(), 
                                          catalog_.currentFile().fileName());
    }
  }

  EventID RootInputFileSequence::seekToEvent(EventID const &eID, MasterProductRegistry& mpr, bool exact) {
    // Attempt to find event in currently open input file.
    bool found = rootFile_->setEntryAtEvent(eID, true);

    // found in the current file
    if( found ) return rootFile_->eventIDForFileIndexPosition(); 

     // fail if not searchable
    if( !catalog_.isSearchable() ) return EventID();

    // Look for event in files previously opened without reopening unnecessary files.
    typedef vector<std::shared_ptr<FileIndex> >::const_iterator Iter;

    for ( Iter itBegin = fileIndexes_.begin(), itEnd = fileIndexes_.end(), it = itBegin; 
          (!found) && it != itEnd; 
          ++it) 
    {
      if (*it && (*it)->containsEvent(eID, exact)) 
      {
        // We found it. Close the currently open file, and open the correct one.
        catalog_.rewindTo( std::distance(itBegin, it) );
        initFile(false);
        if (rootFile_ && primarySequence_) mergeMPR(mpr);

        // Now get the event from the correct file.
        found = rootFile_->setEntryAtEvent(eID, exact);
        assert (found);

        return rootFile_->eventIDForFileIndexPosition();
      }
    }

    // Look for event in files not yet opened.
    while ( catalog_.getNextFile() )
    {
      initFile(false);
      found = rootFile_->setEntryAtEvent(eID, exact);
    }
 
    return (found)?rootFile_->eventIDForFileIndexPosition():EventID();
  }

  EventID RootInputFileSequence::seekToEvent(off_t offset, MasterProductRegistry& mpr, bool) {
    skip(offset, mpr);
    return rootFile_->eventIDForFileIndexPosition();
  }

  vector<FileCatalogItem> const&
  RootInputFileSequence::fileCatalogItems() const {
    return catalog_.fileCatalogItems();
  }

  void
  RootInputFileSequence::endJob() {
    closeFile_();
  }

  std::shared_ptr<FileBlock>
  RootInputFileSequence::readFile_(MasterProductRegistry& mpr) {
    if (firstFile_) {
      // The first input file has already been opened, or a rewind has occurred.
      firstFile_ = false;
      if (!rootFile_) {
        initFile(skipBadFiles_);
      }
    } else {
      if (!nextFile(mpr)) {
        assert(0);
      }
    }
    if (!rootFile_) {
      return std::shared_ptr<FileBlock>(new FileBlock);
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
    std::shared_ptr<TFile> filePtr;
    try {
      logFileAction("  Initiating request to open file ", catalog_.currentFile().fileName());
      filePtr.reset(TFile::Open(catalog_.currentFile().fileName().c_str()));
    }
    catch (cet::exception e) {
      if (!skipBadFiles) {
        throw art::Exception(art::errors::FileOpenError) << e.explain_self() << "\n" <<
          "RootInputFileSequence::initFile(): Input file " << catalog_.currentFile().fileName() << " was not found or could not be opened.\n";
      }
    }
    if (filePtr && !filePtr->IsZombie()) {
      logFileAction("  Successfully opened file ", catalog_.currentFile().fileName());
      rootFile_ = RootInputFileSharedPtr(new RootInputFile(catalog_.currentFile().fileName(), catalog_.url(),
                                                           processConfiguration(), catalog_.currentFile().logicalFileName(), filePtr,
                                                           origEventID_, eventsToSkip_, whichSubRunsToSkip_,
                                                           fastCloningInfo_, treeCacheSize_, treeMaxVirtualSize_,
                                                           processingMode_,
                                                           forcedRunOffset_, eventsToProcess_, noEventSort_,
                                                           groupSelectorRules_, !primarySequence_, duplicateChecker_, dropDescendants_));
      if( catalog_.currentPosition() >= (int)fileIndexes_.size() )
        fileIndexes_.resize( catalog_.currentPosition() );
      fileIndexes_[catalog_.currentPosition()] = rootFile_->fileIndexSharedPtr();
    } else {
      if (!skipBadFiles) {
        throw art::Exception(art::errors::FileOpenError) <<
          "RootInputFileSequence::initFile(): Input file " << catalog_.currentFile().fileName() << " was not found or could not be opened.\n";
      }
      mf::LogWarning("")
        << "Input file: " << catalog_.currentFile().fileName()
        << " was not found or could not be opened, and will be skipped.\n";
    }
  }

  bool RootInputFileSequence::nextFile(MasterProductRegistry& mpr) 
  {
    if( !catalog_.getNextFile() )
    {
      // no more files
      if( primarySequence_ ) 
      {
        return false;
      }

      // rewind only if searchable
      if( catalog_.isSearchable() ) 
      {
        catalog_.rewind();
      }
      else
      {
        return false;
      }
    }

    initFile(skipBadFiles_);

    if (primarySequence_ && rootFile_) mergeMPR(mpr);
    return true;
  }

  bool RootInputFileSequence::previousFile(MasterProductRegistry& mpr) 
  {
    // no going back for non-persistent files
    if( !catalog_.isSearchable() ) return false;

    // no file in the catalog
    if( catalog_.currentPosition()==-1 ) return false;
    
    // first file in the catalog, move to the last file in the list
    if( catalog_.currentPosition()==0 )
    {
      if( primarySequence_ ) return false;

      while( catalog_.getNextFile() ) ;
    }
    else
    {
      catalog_.rewindTo(catalog_.currentPosition()-1);
    }

    initFile(false);

    if (rootFile_) {
      if (primarySequence_) mergeMPR(mpr);
      rootFile_->setToLastEntry();
    }
    return true;
  }

  RootInputFileSequence::~RootInputFileSequence() {
  }

  std::shared_ptr<RunPrincipal>
  RootInputFileSequence::readRun_() {
    return rootFile_->readRun();
  }

  std::shared_ptr<SubRunPrincipal>
  RootInputFileSequence::readSubRun_(std::shared_ptr<RunPrincipal> rp) {
    return rootFile_->readSubRun(rp);
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

  unique_ptr<EventPrincipal>
  RootInputFileSequence::readEvent_() {
    rootFileForLastReadEvent_ = rootFile_;
    return rootFile_->readEvent();
  }

  unique_ptr<EventPrincipal>
  RootInputFileSequence::readCurrentEvent() {
    rootFileForLastReadEvent_ = rootFile_;
    return rootFile_->readCurrentEvent();
  }

  unique_ptr<EventPrincipal>
  RootInputFileSequence::readIt(EventID const& id, MasterProductRegistry& /*mpr*/, bool exact) {
    // Attempt to find event in currently open input file.
    bool found = rootFile_->setEntryAtEvent(id, exact);

    if( found )
    {
      rootFileForLastReadEvent_ = rootFile_;
      unique_ptr<EventPrincipal> eptr(readEvent_());
      return eptr;
    }

    if( !catalog_.isSearchable() )
    {
      return unique_ptr<EventPrincipal>();
    }

    // Look for event in cached files
    typedef vector<std::shared_ptr<FileIndex> >::const_iterator Iter;
    for ( Iter itBegin = fileIndexes_.begin(), itEnd = fileIndexes_.end(), it = itBegin; 
          it != itEnd; 
          ++it) 
    {
      if (*it && (*it)->containsEvent(id, exact)) 
      {
        // We found it. Close the currently open file, and open the correct one.
        catalog_.rewindTo( std::distance(itBegin, it) );
        initFile(false);
        // Now get the event from the correct file.
        found = rootFile_->setEntryAtEvent(id, exact);
        assert (found);
        rootFileForLastReadEvent_ = rootFile_;
        unique_ptr<EventPrincipal> ep = readEvent_();
        return std::move(ep);
      }
    }

    // Look for event in files not yet opened.
    while( catalog_.getNextFile() )
    {
      initFile(false);
      found = rootFile_->setEntryAtEvent(id, exact);
      if (found) 
      {
        rootFileForLastReadEvent_ = rootFile_;
        unique_ptr<EventPrincipal> ep(readEvent_());
        return std::move(ep);
      }
    }

    // Not found
    return unique_ptr<EventPrincipal>();
  }

  std::shared_ptr<SubRunPrincipal>
  RootInputFileSequence::readIt(SubRunID const& id, std::shared_ptr<RunPrincipal> rp) {

    // Attempt to find subRun in currently open input file.
    bool found = rootFile_->setEntryAtSubRun(id);

    if( found ) 
      return readSubRun_(rp);

    if( !catalog_.isSearchable() )
      return std::shared_ptr<SubRunPrincipal>();

    // Look for event in cached files
    typedef vector<std::shared_ptr<FileIndex> >::const_iterator Iter;
    for ( Iter itBegin = fileIndexes_.begin(), itEnd = fileIndexes_.end(), it = itBegin; 
          it != itEnd; 
          ++it) 
    {
      if (*it && (*it)->containsSubRun(id, true)) 
      {
        // We found it. Close the currently open file, and open the correct one.
        catalog_.rewindTo( std::distance(itBegin, it) );
        initFile(false);
        // Now get the subRun from the correct file.
        found = rootFile_->setEntryAtSubRun(id);
        assert (found);
        return readSubRun_(rp);
      }
    }

    // Look for subRun in files not yet opened.
    while( catalog_.getNextFile() )
    {
      initFile(false);
      found = rootFile_->setEntryAtSubRun(id);
      if (found) return readSubRun_(rp);
    }

    // not found
    return std::shared_ptr<SubRunPrincipal>();
  }

  std::shared_ptr<RunPrincipal>
  RootInputFileSequence::readIt(RunID const& id) {

    // Attempt to find run in currently open input file.
    bool found = rootFile_->setEntryAtRun(id);

    if( found ) 
      return readRun_();

    if( !catalog_.isSearchable() )
      return std::shared_ptr<RunPrincipal>();

    // Look for event in cached files
    typedef vector<std::shared_ptr<FileIndex> >::const_iterator Iter;
    for ( Iter itBegin = fileIndexes_.begin(), itEnd = fileIndexes_.end(), it = itBegin; 
          it != itEnd; 
          ++it) 
    {
      if (*it && (*it)->containsRun(id, true)) 
      {
        // We found it. Close the currently open file, and open the correct one.
        catalog_.rewindTo( std::distance(itBegin, it) );
        initFile(false);
        // Now get the event from the correct file.
        found = rootFile_->setEntryAtRun(id);
        assert (found);
        return readRun_();
      }
    }

    // Look for run in files not yet opened.
    while( catalog_.getNextFile() )
    {
      initFile(false);
      found = rootFile_->setEntryAtRun(id);
      if (found) return readRun_();
    }

    // Not found
    return std::shared_ptr<RunPrincipal>();
  }

  input::ItemType
  RootInputFileSequence::getNextItemType() {

    // marked as the first file but failed to find a valid
    // root file. we should make it stop.
    if( firstFile_ && !rootFile_ )
    {
      return input::IsStop; 
    }

    if (firstFile_) {
      return input::IsFile;
    }
    if (rootFile_) {
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

    // now we are either at the end of a root file
    // or the current file is not a root file
    if( !catalog_.hasNextFile() ) 
      return input::IsStop;

    return input::IsFile;
  }

  // Rewind to before the first event that was read.
  void
  RootInputFileSequence::rewind_() {
    if( !catalog_.isSearchable() )
    {
      throw art::Exception(errors::FileOpenError) 
            << "RootInputFileSequence::rewind_() "
            << "cannot rollback on non-searchable file catalogs.";
    }

    firstFile_ = true;
    catalog_.rewind();
    if (duplicateChecker_.get() != 0) duplicateChecker_->rewind();
  }

  // Rewind to the beginning of the current file
  void
  RootInputFileSequence::rewindFile() {
    rootFile_->rewind();
  }

  // Advance "offset" events.  Offset can be positive or negative (or zero).
  void
  RootInputFileSequence::skip(int offset, MasterProductRegistry& mpr) {
    while (offset != 0) {
      offset = rootFile_->skipEvents(offset);
      if (offset > 0 && !nextFile(mpr)) return;
      if (offset < 0 && !previousFile(mpr)) return;
    }
    rootFile_->skipEvents(0);
  }

  bool
  RootInputFileSequence::primary() const {
    return true;
  }

  ProcessConfiguration const&
  RootInputFileSequence::processConfiguration() const {
    return processConfiguration_;
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

  void
  RootInputFileSequence::mergeMPR(MasterProductRegistry &mpr) {
    // make sure the new product registry is compatible with the main one
    string mergeInfo = mpr.merge(rootFile_->productList(),
                                 catalog_.currentFile().fileName(),
                                 matchMode_);
    if (!mergeInfo.empty()) {
      throw art::Exception(errors::MismatchedInputFiles,"RootInputFileSequence::mergeMPR()") << mergeInfo;
    }
    BranchIDListHelper::updateFromInput(rootFile_->branchIDLists(), catalog_.currentFile().fileName());
  }

}  // art

// ======================================================================
