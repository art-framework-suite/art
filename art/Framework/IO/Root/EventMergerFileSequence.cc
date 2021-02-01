#include "art/Framework/IO/Root/EventMergerFileSequence.h"
// vim: set sw=2:

#include "TFile.h"
#include "art/Framework/IO/Root/RootFileBlock.h"
#include "art/Framework/IO/Root/RootInputFile.h"
#include "art/Framework/IO/Root/RootInputTree.h"
#include "art/Framework/IO/Root/mergeProcessHistories.h"
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

  EventMergerFileSequence::EventMergerFileSequence(
    fhicl::TableFragment<Config> const& config,
    FastCloningInfoProvider const& fcip,
    InputSource::ProcessingMode pMode,
    MasterProductRegistry& mpr,
    ProcessConfiguration const& processConfig)
    : fileHandler_{config().fileHandler()}
    , eventsToSkip_{config().skipEvents()}
    , compactSubRunRanges_{config().compactSubRunRanges()}
    , noEventSort_{config().noEventSort()}
    , skipBadFiles_{config().skipBadFiles()}
    , treeCacheSize_{config().cacheSize()}
    , treeMaxVirtualSize_{config().treeMaxVirtualSize()}
    , saveMemoryObjectThreshold_{config().saveMemoryObjectThreshold()}
    , delayedReadEventProducts_{config().delayedReadEventProducts()}
    , delayedReadSubRunProducts_{config().delayedReadSubRunProducts()}
    , delayedReadRunProducts_{config().delayedReadRunProducts()}
    , groupSelectorRules_{config().inputCommands(),
                          "inputCommands",
                          "InputSource"}
    , dropDescendants_{config().dropDescendantsOfDroppedBranches()}
    , readParameterSets_{config().readParameterSets()}
    , fastCloningInfo_{fcip}
    , processingMode_{pMode}
    , processConfiguration_{processConfig}
    , mpr_{mpr}
  {
    RunNumber_t firstRun{};
    bool const haveFirstRun{config().hasFirstRun(firstRun)};
    SubRunNumber_t firstSubRun{};
    bool const haveFirstSubRun{config().hasFirstSubRun(firstSubRun)};
    EventNumber_t firstEvent{};
    bool const haveFirstEvent{config().hasFirstEvent(firstEvent)};

    RunID const firstRunID{haveFirstRun ? RunID{firstRun} : RunID::firstRun()};
    SubRunID const firstSubRunID{haveFirstSubRun ?
                                   SubRunID{firstRunID.run(), firstSubRun} :
                                   SubRunID::firstSubRun(firstRunID)};

    origEventID_ = haveFirstEvent ? EventID{firstSubRunID, firstEvent} :
                                    EventID::firstEvent(firstSubRunID);

    if (noEventSort_ && haveFirstEvent) {
      throw art::Exception(errors::Configuration)
        << "Illegal configuration options passed to RootInput\n"
        << "You cannot request \"noEventSort\" and also set \"firstEvent\".\n";
    }

    duplicateChecker_ = std::make_shared<DuplicateChecker>(config().dc);
    while (fileHandler_.getNextFile()) {
      initFiles(skipBadFiles_);
      if (primaryFile_) {
        // We found one, good, stop now.
        break;
      }
    }
    if (!primaryFile_) {
      // We could not open any input files, stop.
      return;
    }
    if (config().setRunNumber(setRun_)) {
      try {
        forcedRunOffset_ = primaryFile_->setForcedRunOffset(setRun_);
      }
      catch (art::Exception& e) {
        if (e.categoryCode() == errors::InvalidNumber) {
          throw Exception(errors::Configuration)
            << "setRunNumber " << setRun_
            << " does not correspond to a valid run number in ["
            << RunID::firstRun().run() << ", " << RunID::maxRun().run()
            << "]\n";
        } else {
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
        << "Source parameter readParameterSets was set to false: parameter set "
           "provenance\n"
        << "will NOT be available in this or subsequent jobs using output from "
           "this job.\n"
        << "Check your experiment's policy on this issue to avoid future "
           "problems\n"
        << "with analysis reproducibility.\n";
    }
    if (compactSubRunRanges_) {
      mf::LogWarning("PROVENANCE")
        << "Source parameter compactEventRanges was set to true: enabling "
           "compact event ranges\n"
        << "creates a history that can cause file concatenation problems if a "
           "given SubRun spans\n"
        << "multiple input files.  Use with care.\n";
    }
  }

  void
  EventMergerFileSequence::endJob()
  {
    closeFiles_();
  }

  std::unique_ptr<FileBlock>
  EventMergerFileSequence::readFile_()
  {
    if (firstFile_) {
      // We are at the first file in the sequence of files.
      firstFile_ = false;
      if (!primaryFile_) {
        initFiles(skipBadFiles_);
      }
    } else if (!nextFile()) {
      // FIXME: Turn this into a throw!
      assert(false);
    }
    if (!primaryFile_) {
      return std::make_unique<RootFileBlock>();
    }
    return primaryFile_->createFileBlock();
  }

  void
  EventMergerFileSequence::closeFiles_()
  {
    if (!primaryFile_)
      return;

    // Account for events skipped in the file.
    eventsToSkip_ = primaryFile_->eventsToSkip();
    primaryFile_->close();

    for (auto& sf : filesToMergeWithPrimary_) {
      sf->close();
    }

    detail::logFileAction("Closed input file ", primaryFile_->fileName());
    primaryFile_.reset();
    if (duplicateChecker_) {
      duplicateChecker_->inputFileClosed();
    }
  }

  void
  EventMergerFileSequence::initFiles(bool const skipBadFiles)
  {
    closeFiles_();
    primaryFile_ =
      openFile(fileHandler_.currentFileName(), skipBadFiles, duplicateChecker_);
    for (auto const& sf_name : fileHandler_.currentSecondaryFileNames()) {
      auto sf = openFile(sf_name, skipBadFiles);
      if (not sf) {
        continue;
      }
      filesToMergeWithPrimary_.push_back(move(sf));
    }
  }

  std::shared_ptr<RootInputFile>
  EventMergerFileSequence::openFile(
    std::string const& fileName,
    bool const skipBadFiles,
    std::shared_ptr<DuplicateChecker> duplicateChecker)
  {
    std::unique_ptr<TFile> filePtr;
    try {
      detail::logFileAction("Initiating request to open input file ", fileName);
      filePtr.reset(TFile::Open(fileName.c_str()));
    }
    catch (cet::exception e) {
      if (!skipBadFiles) {
        throw art::Exception(art::errors::FileOpenError)
          << e.explain_self()
          << "\nEventMergerFileSequence::openFile(): Input file " << fileName
          << " was not found or could not be opened.\n";
      }
    }
    if (!filePtr || filePtr->IsZombie()) {
      if (!skipBadFiles) {
        throw art::Exception(art::errors::FileOpenError)
          << "EventMergerFileSequence::openFile(): Input file " << fileName
          << " was not found or could not be opened.\n";
      }
      mf::LogWarning("")
        << "Input file: " << fileName
        << " was not found or could not be opened, and will be skipped.\n";
      return nullptr;
    }
    detail::logFileAction("Opened input file ", fileName);
    return std::make_shared<RootInputFile>(fileName,
                                           processConfiguration(),
                                           std::move(filePtr),
                                           origEventID_,
                                           eventsToSkip_,
                                           compactSubRunRanges_,
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
                                           dropDescendants_,
                                           readParameterSets_,
                                           mpr_,
                                           duplicateChecker);
  }

  bool
  EventMergerFileSequence::nextFile()
  {
    if (!fileHandler_.getNextFile()) {
      // no more files
      return false;
    }
    initFiles(skipBadFiles_);
    return true;
  }

  unique_ptr<EventPrincipal>
  EventMergerFileSequence::readIt(EventID const& id, bool exact)
  {
    if (primaryFile_->setEntry(InEvent, id, exact)) {
      rootFileForLastReadEvent_ = primaryFile_;
      return readEvent_();
    }
    return nullptr;
  }

  unique_ptr<EventPrincipal>
  EventMergerFileSequence::readEvent_()
  {
    // Create and setup the EventPrincipal.
    //
    //   1. create an EventPrincipal with a unique EventID
    //   2. For each entry in the provenance, put in one Group,
    //      holding the Provenance for the corresponding EDProduct.
    //   3. set up the caches in the EventPrincipal to know about this
    //      Group.
    //
    // We do *not* create the EDProduct instance (the equivalent of
    // reading the branch containing this EDProduct). That will be
    // done by the Delayed Reader when it is asked to do so.
    //
    rootFileForLastReadEvent_ = primaryFile_;

    auto primary_ep = primaryFile_->readEvent();
    auto primary_history = primary_ep->processHistory();

    // Read all secondary principals into memory
    vector<ProcessHistory> secondary_histories;
    for (auto& sf : filesToMergeWithPrimary_) {
      auto ep = sf->readEventWithID(primary_ep->id());
      secondary_histories.push_back(ep->processHistory());
      primary_ep->addSecondaryPrincipal(move(ep));
    }

    primary_ep->setProcessHistory(
      merge_process_histories(primary_history, secondary_histories));
    return primary_ep;
  }

  std::unique_ptr<RangeSetHandler>
  EventMergerFileSequence::runRangeSetHandler()
  {
    return primaryFile_->runRangeSetHandler();
  }

  std::unique_ptr<RangeSetHandler>
  EventMergerFileSequence::subRunRangeSetHandler()
  {
    return primaryFile_->subRunRangeSetHandler();
  }

  std::unique_ptr<SubRunPrincipal>
  EventMergerFileSequence::readIt(SubRunID const& id)
  {
    if (primaryFile_->setEntry(InSubRun, id)) {
      return readSubRun_();
    }
    return std::unique_ptr<SubRunPrincipal>{nullptr};
  }

  std::unique_ptr<SubRunPrincipal>
  EventMergerFileSequence::readSubRun_()
  {
    auto primary_srp = primaryFile_->readSubRun();
    auto primary_history = primary_srp->processHistory();

    // Read all secondary principals into memory
    vector<ProcessHistory> secondary_histories;
    for (auto& sf : filesToMergeWithPrimary_) {
      auto srp = sf->readSubRunWithID(primary_srp->id());
      secondary_histories.push_back(srp->processHistory());
      primary_srp->addSecondaryPrincipal(move(srp));
    }

    primary_srp->setProcessHistory(
      merge_process_histories(primary_history, secondary_histories));
    return primary_srp;
  }

  std::unique_ptr<RunPrincipal>
  EventMergerFileSequence::readIt(RunID const& id)
  {
    if (primaryFile_->setEntry(InRun, id)) {
      return readRun_();
    }
    return std::unique_ptr<RunPrincipal>{nullptr};
  }

  std::unique_ptr<RunPrincipal>
  EventMergerFileSequence::readRun_()
  {
    auto primary_rp = primaryFile_->readRun();
    auto primary_history = primary_rp->processHistory();

    // Read all secondary principals into memory
    vector<ProcessHistory> secondary_histories;
    for (auto& sf : filesToMergeWithPrimary_) {
      auto rp = sf->readRunWithID(primary_rp->id());
      secondary_histories.push_back(rp->processHistory());
      primary_rp->addSecondaryPrincipal(move(rp));
    }

    primary_rp->setProcessHistory(
      merge_process_histories(primary_history, secondary_histories));
    return primary_rp;
  }

  input::ItemType
  EventMergerFileSequence::getNextItemType()
  {
    // marked as the first file but failed to find a valid root
    // file. we should make it stop.
    if (firstFile_ && !primaryFile_) {
      return input::IsStop;
    }
    if (firstFile_) {
      return input::IsFile;
    }
    if (primaryFile_) {
      FileIndex::EntryType entryType = primaryFile_->getNextEntryTypeWanted();
      if (entryType == FileIndex::kEvent) {
        return input::IsEvent;
      } else if (entryType == FileIndex::kSubRun) {
        return input::IsSubRun;
      } else if (entryType == FileIndex::kRun) {
        return input::IsRun;
      }
      assert(entryType == FileIndex::kEnd);
    }
    // Now we are either at the end of a root file or the current file
    // is not a root file
    if (!fileHandler_.hasNextFile()) {
      return input::IsStop;
    }
    return input::IsFile;
  }

  ProcessConfiguration const&
  EventMergerFileSequence::processConfiguration() const
  {
    return processConfiguration_;
  }

} // namespace art
