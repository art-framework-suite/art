#include "art/Framework/IO/Root/RootInputFileSequence.h"
// vim: set sw=2:

#include "TFile.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/IO/Catalog/FileCatalog.h"
#include "art/Framework/IO/Catalog/InputFileCatalog.h"
#include "art/Framework/IO/Root/RootInputFile.h"
#include "art/Framework/IO/Root/RootInputTree.h"
#include "art/Framework/IO/detail/logFileAction.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <ctime>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <utility>

using namespace cet;
using namespace std;

namespace art {

RootInputFileSequence::
RootInputFileSequence(fhicl::TableFragment<RootInputFileSequence::Config> const& config,
                      InputFileCatalog& catalog,
                      FastCloningInfoProvider const& fcip,
                      InputSource::ProcessingMode pMode,
                      MasterProductRegistry& mpr,
                      ProcessConfiguration const& processConfig)
  : catalog_{catalog}
  , fileIndexes_(fileCatalogItems().size())
  , eventsToSkip_{config().skipEvents()}
  , noEventSort_{config().noEventSort()}
  , skipBadFiles_{config().skipBadFiles()}
  , treeCacheSize_{config().cacheSize()}
  , treeMaxVirtualSize_{config().treeMaxVirtualSize()}
  , saveMemoryObjectThreshold_{config().saveMemoryObjectThreshold()}
  , delayedReadEventProducts_{config().delayedReadEventProducts()}
  , delayedReadSubRunProducts_{config().delayedReadSubRunProducts()}
  , delayedReadRunProducts_{config().delayedReadRunProducts()}
  , groupSelectorRules_{config().inputCommands(), "inputCommands", "InputSource"}
  , dropDescendants_{config().dropDescendantsOfDroppedBranches()}
  , readParameterSets_{config().readParameterSets()}
  , fastCloningInfo_{fcip}
  , processingMode_{pMode}
  , processConfiguration_{processConfig}
  , mpr_{mpr}
{
  auto const& primaryFileNames = catalog_.fileSources();

  map<string const, vector<string> const> secondaryFilesMap;

  std::vector<Config::SecondaryFile> secondaryFiles;
  if (config().secondaryFileNames(secondaryFiles)) {
    for (auto const& val: secondaryFiles) {
      auto const a = val.a();
      auto const b = val.b();
      if (a.empty()) {
        throw art::Exception(errors::Configuration)
          << "Empty filename found as value of an \"a\" parameter!\n";
      }
      for (auto const& name: b) {
        if (name.empty()) {
          throw art::Exception(errors::Configuration)
            << "Empty secondary filename found as value of an \"b\" parameter!\n";
        }
      }
      secondaryFilesMap.emplace(a, b);
    }
  }

  vector<pair<vector<string>::const_iterator,
         vector<string>::const_iterator>> stk;
  for (auto const& primaryFileName : primaryFileNames) {
    vector<string> secondaries;
    auto SFMI = secondaryFilesMap.find(primaryFileName);
    if (SFMI == secondaryFilesMap.end()) {
      // This primary has no secondaries.
      secondaryFileNames_.push_back(std::move(secondaries));
      continue;
    }
    if (!SFMI->second.size()) {
      // Has an empty secondary list.
      secondaryFileNames_.push_back(std::move(secondaries));
      continue;
    }
    stk.emplace_back(SFMI->second.cbegin(), SFMI->second.cend());
    while (stk.size()) {
      auto val = stk.back();
      stk.pop_back();
      if (val.first == val.second) {
        // Reached end of this filename list.
        continue;
      }
      auto const& fn = *val.first;
      ++val.first;
      secondaries.push_back(fn);
      auto SI = secondaryFilesMap.find(fn);
      if (SI == secondaryFilesMap.end()) {
        // Has no secondary list.
        if (val.first == val.second) {
          // Reached end of this filename list.
          continue;
        }
        stk.emplace_back(val.first, val.second);
        continue;
      }
      if (!SI->second.size()) {
        // Has an empty secondary list.
        if (val.first == val.second) {
          // Reached end of this filename list.
          continue;
        }
        stk.emplace_back(val.first, val.second);
        continue;
      }
      stk.emplace_back(val.first, val.second);
      stk.emplace_back(SI->second.cbegin(), SI->second.cend());
    }
    secondaryFileNames_.push_back(std::move(secondaries));
  }
  RunNumber_t firstRun {};
  bool const haveFirstRun {config().hasFirstRun(firstRun)};
  SubRunNumber_t firstSubRun {};
  bool const haveFirstSubRun {config().hasFirstSubRun(firstSubRun)};
  EventNumber_t firstEvent {};
  bool const haveFirstEvent {config().hasFirstEvent(firstEvent)};

  RunID const firstRunID {haveFirstRun ? RunID{firstRun} : RunID::firstRun()};
  SubRunID const firstSubRunID {haveFirstSubRun ? SubRunID{firstRunID.run(), firstSubRun} : SubRunID::firstSubRun(firstRunID)};

  origEventID_ = haveFirstEvent ? EventID{firstSubRunID, firstEvent} : EventID::firstEvent(firstSubRunID);

  if (noEventSort_ && haveFirstEvent) {
    throw art::Exception(errors::Configuration)
        << "Illegal configuration options passed to RootInput\n"
        << "You cannot request \"noEventSort\" and also set \"firstEvent\".\n";
  }
  if (primary()) {
    duplicateChecker_ = std::make_shared<DuplicateChecker>(config().dc);
  }
  if (pendingClose_) {
    throw Exception(errors::LogicError)
      << "RootInputFileSequence looking for next file with a pending close!";
  }
  while (catalog_.getNextFile()) {
    initFile(skipBadFiles_);
    if (rootFile_) {
      // We found one, good, stop now.
      break;
    }
  }
  if (!rootFile_) {
    // We could not open any input files, stop.
    return;
  }
  if (config().setRunNumber(setRun_)) {
    try {
      forcedRunOffset_ = rootFile_->setForcedRunOffset(setRun_);
    }
    catch (art::Exception& e) {
      if (e.categoryCode() == errors::InvalidNumber) {
        throw Exception(errors::Configuration)
            << "setRunNumber "
            << setRun_
            << " does not correspond to a valid run number in ["
            << RunID::firstRun().run()
            << ", "
            << RunID::maxRun().run()
            << "]\n";
      }
      else {
        throw; // Rethrow.
      }
    }
    if (forcedRunOffset_ < 0) {
      throw art::Exception(errors::Configuration)
          << "The value of the 'setRunNumber' parameter must not be\n"
          << "less than the first run number in the first input file.\n"
          << "'setRunNumber' was " << setRun_ << ", while the first run was "
          << setRun_ - forcedRunOffset_ << ".\n";
    }
  }
  if (!readParameterSets_) {
    mf::LogWarning("PROVENANCE")
      << "Source parameter readParameterSets was set to false: parameter set provenance\n"
      << "will NOT be available in this or subsequent jobs using output from this job.\n"
      << "Check your experiment's policy on this issue  to avoid future problems\n"
      << "with analysis reproducibility.\n";
  }
}

EventID
RootInputFileSequence::
seekToEvent(EventID const& eID, bool exact)
{
  // Attempt to find event in currently open input file.
  bool found = rootFile_->setEntry<InEvent>(eID, true);
  // found in the current file
  if (found) {
    return rootFile_->eventIDForFileIndexPosition();
  }
  // fail if not searchable
  if (!catalog_.isSearchable()) {
    return EventID();
  }
  // Look for event in files previously opened without reopening unnecessary files.
  for (auto itBegin = fileIndexes_.cbegin(), itEnd = fileIndexes_.cend(),
       it = itBegin;
       (!found) && it != itEnd;
       ++it) {
    if (*it && (*it)->contains(eID, exact)) {
      // We found it. Close the currently open file, and open the correct one.
      catalog_.rewindTo(std::distance(itBegin, it));
      initFile(/*skipBadFiles=*/false);
      // Now get the event from the correct file.
      found = rootFile_->setEntry<InEvent>(eID, exact);
      assert(found);
      return rootFile_->eventIDForFileIndexPosition();
    }
  }
  // Look for event in files not yet opened.
  while (catalog_.getNextFile()) {
    initFile(/*skipBadFiles=*/false);
    found = rootFile_->setEntry<InEvent>(eID, exact);
  }
  return (found) ? rootFile_->eventIDForFileIndexPosition() : EventID();
}

EventID
RootInputFileSequence::
seekToEvent(off_t offset, bool)
{
  skip(offset);
  return rootFile_->eventIDForFileIndexPosition();
}

vector<FileCatalogItem> const&
RootInputFileSequence::
fileCatalogItems() const
{
  return catalog_.fileCatalogItems();
}

void
RootInputFileSequence::
endJob()
{
  closeFile_();
}

std::unique_ptr<FileBlock>
RootInputFileSequence::
readFile_()
{
  if (firstFile_) {
    // We are at the first file in the sequence of files.
    firstFile_ = false;
    if (!rootFile_) {
      initFile(skipBadFiles_);
    }
  }
  else if (!nextFile()) {
    // FIXME: Turn this into a throw!
    assert(false);
  }
  if (!rootFile_) {
    return std::make_unique<FileBlock>();
  }
  return rootFile_->createFileBlock();
}

void
RootInputFileSequence::
closeFile_()
{
  if (pendingClose_) {
    catalog_.finish(); // We were expecting this
    pendingClose_ = false;
  }
  if (!rootFile_) return;

  // Account for events skipped in the file.
  eventsToSkip_ = rootFile_->eventsToSkip();
  rootFile_->close(primary());
  detail::logFileAction("Closed input file ", rootFile_->fileName());
  rootFile_.reset();
  if (duplicateChecker_.get() != nullptr) {
    duplicateChecker_->inputFileClosed();
  }
}

void
RootInputFileSequence::
finish()
{
  pendingClose_ = true;
}

void
RootInputFileSequence::
initFile(bool const skipBadFiles)
{
  // close the currently open file, any, and delete the RootInputFile object.
  closeFile_();
  std::unique_ptr<TFile> filePtr;
  try {
    detail::logFileAction("Initiating request to open input file ",
                          catalog_.currentFile().fileName());
    filePtr.reset(TFile::Open(catalog_.currentFile().fileName().c_str()));
  }
  catch (cet::exception e) {
    if (!skipBadFiles) {
      throw art::Exception(art::errors::FileOpenError)
          << e.explain_self()
          << "\nRootInputFileSequence::initFile(): Input file "
          << catalog_.currentFile().fileName()
          << " was not found or could not be opened.\n";
    }
  }
  if (!filePtr || filePtr->IsZombie()) {
    if (!skipBadFiles) {
      throw art::Exception(art::errors::FileOpenError)
          << "RootInputFileSequence::initFile(): Input file "
          << catalog_.currentFile().fileName()
          << " was not found or could not be opened.\n";
    }
    mf::LogWarning("")
        << "Input file: "
        << catalog_.currentFile().fileName()
        << " was not found or could not be opened, and will be skipped.\n";
    return;
  }
  detail::logFileAction("Opened input file ",
                        catalog_.currentFile().fileName());
  vector<string> empty_vs;
  rootFile_ = make_shared<RootInputFile>(catalog_.currentFile().fileName(),
                                         catalog_.url(),
                                         processConfiguration(),
                                         catalog_.currentFile().logicalFileName(),
                                         std::move(filePtr),
                                         origEventID_,
                                         eventsToSkip_,
                                         fastCloningInfo_,
                                         treeCacheSize_,
                                         treeMaxVirtualSize_,
                                         saveMemoryObjectThreshold_,
                                         delayedReadEventProducts_,
                                         delayedReadSubRunProducts_,
                                         delayedReadRunProducts_,
                                         processingMode_,
                                         forcedRunOffset_,
                                         noEventSort_,
                                         groupSelectorRules_,
                                         duplicateChecker_,
                                         dropDescendants_,
                                         readParameterSets_,
                                         /*primaryFile*/exempt_ptr<RootInputFile>{nullptr},
                                         secondaryFileNames_.empty() ? empty_vs : secondaryFileNames_.at(catalog_.currentIndex()),
                                         this,
                                         mpr_);

  assert(catalog_.currentIndex() != InputFileCatalog::indexEnd);
  if (catalog_.currentIndex() + 1 > fileIndexes_.size()) {
    fileIndexes_.resize(catalog_.currentIndex() + 1);
  }
  fileIndexes_[catalog_.currentIndex()] = rootFile_->fileIndexSharedPtr();
}

std::unique_ptr<RootInputFile>
RootInputFileSequence::
openSecondaryFile(string const& name,
                  exempt_ptr<RootInputFile> primaryFile)
{
  std::unique_ptr<TFile> filePtr;
  try {
    detail::logFileAction("Attempting  to open secondary input file ", name);
    filePtr.reset(TFile::Open(name.c_str()));
  }
  catch (cet::exception e) {
    throw art::Exception(art::errors::FileOpenError)
        << e.explain_self()
        << "\nRootInputFileSequence::openSecondaryFile(): Input file "
        << name
        << " was not found or could not be opened.\n";
  }
  if (!filePtr || filePtr->IsZombie()) {
    throw art::Exception(art::errors::FileOpenError)
        << "RootInputFileSequence::openSecondaryFile(): Input file "
        << name
        << " was not found or could not be opened.\n";
  }
  detail::logFileAction("Opened secondary input file ", name);
  vector<string> empty_secondary_filenames;
  return std::make_unique<RootInputFile>(name,
                                         /*url*/"",
                                         processConfiguration(),
                                         /*logicalFileName*/"",
                                         std::move(filePtr),
                                         origEventID_,
                                         eventsToSkip_,
                                         fastCloningInfo_,
                                         treeCacheSize_,
                                         treeMaxVirtualSize_,
                                         saveMemoryObjectThreshold_,
                                         delayedReadEventProducts_,
                                         delayedReadSubRunProducts_,
                                         delayedReadRunProducts_,
                                         processingMode_,
                                         forcedRunOffset_,
                                         noEventSort_,
                                         groupSelectorRules_,
                                         /*duplicateChecker_*/nullptr,
                                         dropDescendants_,
                                         readParameterSets_,
                                         primaryFile,
                                         empty_secondary_filenames,
                                         this,
                                         mpr_);
}

bool
RootInputFileSequence::
nextFile()
{
  if (!catalog_.getNextFile()) {
    // no more files
    return false;
  }
  initFile(skipBadFiles_);
  return true;
}

bool
RootInputFileSequence::
previousFile()
{
  // no going back for non-persistent files
  if (!catalog_.isSearchable()) {
    return false;
  }
  // no file in the catalog
  if (catalog_.currentIndex() == InputFileCatalog::indexEnd) {
    return false;
  }
  // first file in the catalog, move to the last file in the list
  if (catalog_.currentIndex() == 0) {
    return false;
  }
  else {
    catalog_.rewindTo(catalog_.currentIndex() - 1);
  }
  initFile(/*skipBadFiles=*/false);
  if (rootFile_) {
    rootFile_->setToLastEntry();
  }
  return true;
}

unique_ptr<EventPrincipal>
RootInputFileSequence::
readIt(EventID const& id, bool exact)
{
  // Attempt to find event in currently open input file.
  bool found = rootFile_->setEntry<InEvent>(id, exact);
  if (found) {
    rootFileForLastReadEvent_ = rootFile_;
    unique_ptr<EventPrincipal> eptr(readEvent_());
    return eptr;
  }
  if (!catalog_.isSearchable()) {
    //return unique_ptr<EventPrincipal>();
    return 0;
  }
  // Look for event in cached files
  for (auto IB = fileIndexes_.cbegin(), IE = fileIndexes_.cend(), I = IB;
       I != IE; ++I) {
    if (*I && (*I)->contains(id, exact)) {
      // We found it. Close the currently open file, and open the correct one.
      catalog_.rewindTo(std::distance(IB, I));
      initFile(/*skipBadFiles=*/false);
      // Now get the event from the correct file.
      found = rootFile_->setEntry<InEvent>(id, exact);
      assert(found);
      rootFileForLastReadEvent_ = rootFile_;
      unique_ptr<EventPrincipal> ep = readEvent_();
      return std::move(ep);
    }
  }
  // Look for event in files not yet opened.
  while (catalog_.getNextFile()) {
    initFile(/*skipBadFiles=*/false);
    found = rootFile_->setEntry<InEvent>(id, exact);
    if (found) {
      rootFileForLastReadEvent_ = rootFile_;
      unique_ptr<EventPrincipal> ep(readEvent_());
      return std::move(ep);
    }
  }
  // Not found
  return 0;
}

unique_ptr<EventPrincipal>
RootInputFileSequence::
readEvent_()
{
  // Create and setup the EventPrincipal.
  //
  //   1. create an EventPrincipal with a unique EventID
  //   2. For each entry in the provenance, put in one Group,
  //      holding the Provenance for the corresponding EDProduct.
  //   3. set up the caches in the EventPrincipal to know about this
  //      Group.
  //
  // We do *not* create the EDProduct instance (the equivalent of reading
  // the branch containing this EDProduct. That will be done by the
  // Delayed Reader when it is asked to do so.
  //
  rootFileForLastReadEvent_ = rootFile_;
  return rootFile_->readEvent();
}

std::unique_ptr<RangeSetHandler>
RootInputFileSequence::runRangeSetHandler()
{
  return rootFile_->runRangeSetHandler();
}

std::unique_ptr<RangeSetHandler>
RootInputFileSequence::subRunRangeSetHandler()
{
  return rootFile_->subRunRangeSetHandler();
}

std::unique_ptr<SubRunPrincipal>
RootInputFileSequence::
readIt(SubRunID const& id, cet::exempt_ptr<RunPrincipal> rp)
{
  // Attempt to find subRun in currently open input file.
  bool found = rootFile_->setEntry<InSubRun>(id);
  if (found) {
    return readSubRun_(rp);
  }
  if (!catalog_.isSearchable()) {
    return std::unique_ptr<SubRunPrincipal>{nullptr};
  }
  // Look for event in cached files
  typedef vector<std::shared_ptr<FileIndex>>::const_iterator Iter;
  for (Iter itBegin = fileIndexes_.begin(), itEnd = fileIndexes_.end(),
       it = itBegin;
       it != itEnd;
       ++it) {
    if (*it && (*it)->contains(id, true)) {
      // We found it. Close the currently open file, and open the correct one.
      catalog_.rewindTo(std::distance(itBegin, it));
      initFile(/*skipBadFiles=*/false);
      // Now get the subRun from the correct file.
      found = rootFile_->setEntry<InSubRun>(id);
      assert(found);
      return readSubRun_(rp);
    }
  }
  // Look for subRun in files not yet opened.
  while (catalog_.getNextFile()) {
    initFile(/*skipBadFiles=*/false);
    found = rootFile_->setEntry<InSubRun>(id);
    if (found) {
      return readSubRun_(rp);
    }
  }
  // not found
  return std::unique_ptr<SubRunPrincipal>{nullptr};
}

std::unique_ptr<SubRunPrincipal>
RootInputFileSequence::
readSubRun_(cet::exempt_ptr<RunPrincipal> rp)
{
  return rootFile_->readSubRun(rp);
}

std::unique_ptr<RunPrincipal>
RootInputFileSequence::
readIt(RunID const& id)
{
  // Attempt to find run in current file.
  bool found = rootFile_->setEntry<InRun>(id);
  if (found) {
    // Got it, read the run.
    return readRun_();
  }
  if (!catalog_.isSearchable()) {
    // Cannot random access files, give up.
    return std::unique_ptr<RunPrincipal>{nullptr};
  }
  // Look for the run in the opened files.
  for (auto B = fileIndexes_.cbegin(), E = fileIndexes_.cend(), I = B;
       I != E; ++I) {
    if (*I && (*I)->contains(id, true)) {
      // We found it, open the file.
      catalog_.rewindTo(std::distance(B, I));
      initFile(/*skipBadFiles=*/false);
      // Now read the run.
      found = rootFile_->setEntry<InRun>(id);
      assert(found);
      return readRun_();
    }
  }
  // Look for run in files not yet opened.
  while (catalog_.getNextFile()) {
    initFile(/*skipBadFiles=*/false);
    found = rootFile_->setEntry<InRun>(id);
    if (found) {
      // Got it, read the run.
      return readRun_();
    }
  }
  // Not found.
  return std::unique_ptr<RunPrincipal>{nullptr};
}

std::unique_ptr<RunPrincipal>
RootInputFileSequence::
readRun_()
{
  return rootFile_->readRun();
}

input::ItemType
RootInputFileSequence::
getNextItemType()
{
  // marked as the first file but failed to find a valid
  // root file. we should make it stop.
  if (firstFile_ && !rootFile_) {
    return input::IsStop;
  }
  if (firstFile_) {
    return input::IsFile;
  }
  if (rootFile_) {
    FileIndex::EntryType entryType = rootFile_->getNextEntryTypeWanted();
    if (entryType == FileIndex::kEvent) {
      return input::IsEvent;
    }
    else if (entryType == FileIndex::kSubRun) {
      return input::IsSubRun;
    }
    else if (entryType == FileIndex::kRun) {
      return input::IsRun;
    }
    assert(entryType == FileIndex::kEnd);
  }
  // now we are either at the end of a root file
  // or the current file is not a root file
  if (!catalog_.hasNextFile()) {
    return input::IsStop;
  }
  return input::IsFile;
}

// Rewind to before the first event that was read.
void
RootInputFileSequence::
rewind_()
{
  if (!catalog_.isSearchable()) {
    throw art::Exception(errors::FileOpenError)
        << "RootInputFileSequence::rewind_() "
        << "cannot rollback on non-searchable file catalogs.";
  }
  firstFile_ = true;
  catalog_.rewind();
  if (duplicateChecker_.get() != nullptr) {
    duplicateChecker_->rewind();
  }
}

// Rewind to the beginning of the current file
void
RootInputFileSequence::
rewindFile()
{
  rootFile_->rewind();
}

// Advance "offset" events.  Offset can be positive or negative (or zero).
void
RootInputFileSequence::
skip(int offset)
{
  while (offset != 0) {
    offset = rootFile_->skipEvents(offset);
    if (offset > 0 && !nextFile()) {
      return;
    }
    if (offset < 0 && !previousFile()) {
      return;
    }
  }
  rootFile_->skipEvents(0);
}

bool
RootInputFileSequence::
primary() const
{
  return true;
}

ProcessConfiguration const&
RootInputFileSequence::
processConfiguration() const
{
  return processConfiguration_;
}

} // namespace art
