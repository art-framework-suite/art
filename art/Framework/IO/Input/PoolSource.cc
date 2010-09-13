/*----------------------------------------------------------------------
----------------------------------------------------------------------*/


#include "PoolSource.h"

#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/ParameterSet/ParameterSetDescription.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Utilities/EDMException.h"
#include "art/Utilities/Exception.h"

#include "RootInputFileSequence.h"
#include "TTreeCache.h"

#include <set>


namespace edm {

  class SubRunID;
  class EventID;

  namespace {
    void checkHistoryConsistency(Principal const& primary, Principal const& secondary) {
      ProcessHistory const& ph1 = primary.processHistory();
      ProcessHistory const& ph2 = secondary.processHistory();
      if (ph1 != ph2 && !isAncestor(ph2, ph1)) {
        throw edm::Exception(errors::MismatchedInputFiles, "PoolSource::checkConsistency") <<
          "The secondary file is not an ancestor of the primary file\n";
      }
    }
    void checkConsistency(EventPrincipal const& primary, EventPrincipal const& secondary) {
      if (!isSameEvent(primary, secondary)) {
        throw edm::Exception(errors::MismatchedInputFiles, "PoolSource::checkConsistency") <<
          primary.id() << " has inconsistent EventAuxiliary data in the primary and secondary file\n";
      }
      checkHistoryConsistency(primary, secondary);
    }
    void checkConsistency(SubRunPrincipal const& primary, SubRunPrincipal const& secondary) {
      if (primary.id() != secondary.id()) {
        throw edm::Exception(errors::MismatchedInputFiles, "PoolSource::checkConsistency") <<
          primary.id() << " has inconsistent SubRunAuxiliary data in the primary and secondary file\n";
      }
      checkHistoryConsistency(primary, secondary);
    }
    void checkConsistency(RunPrincipal const& primary, RunPrincipal const& secondary) {
      if (primary.id() != secondary.id()) {
        throw edm::Exception(errors::MismatchedInputFiles, "PoolSource::checkConsistency") <<
          primary.id() << " has inconsistent RunAuxiliary data in the primary and secondary file\n";
      }
      checkHistoryConsistency(primary, secondary);
    }
  }

  PoolSource::PoolSource(fhicl::ParameterSet const& pset, InputSourceDescription const& desc) :
    VectorInputSource(pset, desc),
    rootServiceChecker_(),
    primaryFileSequence_(new RootInputFileSequence(pset, *this, catalog(), primary())),
    secondaryFileSequence_(catalog(1).empty() ? 0 : new RootInputFileSequence(pset, *this, catalog(1), false)),
    branchIDsToReplace_() {
    if (secondaryFileSequence_) {
      boost::array<std::set<BranchID>, NumBranchTypes> idsToReplace;
      ProductRegistry::ProductList const& secondary = secondaryFileSequence_->fileProductRegistry().productList();
      ProductRegistry::ProductList const& primary = primaryFileSequence_->fileProductRegistry().productList();
      typedef ProductRegistry::ProductList::const_iterator const_iterator;
      for (const_iterator it = secondary.begin(), itEnd = secondary.end(); it != itEnd; ++it) {
        if (it->second.present()) idsToReplace[it->second.branchType()].insert(it->second.branchID());
      }
      for (const_iterator it = primary.begin(), itEnd = primary.end(); it != itEnd; ++it) {
        if (it->second.present()) idsToReplace[it->second.branchType()].erase(it->second.branchID());
      }
      if (idsToReplace[InEvent].empty() && idsToReplace[InSubRun].empty() && idsToReplace[InRun].empty()) {
        secondaryFileSequence_.reset();
      }
      else {
        for (int i = InEvent; i < NumBranchTypes; ++i) {
          branchIDsToReplace_[i].reserve(idsToReplace[i].size());
          for (std::set<BranchID>::const_iterator it = idsToReplace[i].begin(), itEnd = idsToReplace[i].end();
               it != itEnd; ++it) {
            branchIDsToReplace_[i].push_back(*it);
          }
        }
      }
    }
  }

  PoolSource::~PoolSource() {}

  void
  PoolSource::endJob() {
    if (secondaryFileSequence_) secondaryFileSequence_->endJob();
    primaryFileSequence_->endJob();
  }

  boost::shared_ptr<FileBlock>
  PoolSource::readFile_() {
    if (secondaryFileSequence_) {
        boost::shared_ptr<FileBlock> fb = primaryFileSequence_->readFile_();
        fb->setNotFastCopyable();
        return fb;
    }
    return primaryFileSequence_->readFile_();
  }

  void PoolSource::closeFile_() {
    primaryFileSequence_->closeFile_();
  }

  boost::shared_ptr<RunPrincipal>
  PoolSource::readRun_() {
    if (secondaryFileSequence_ && !branchIDsToReplace_[InRun].empty()) {
      boost::shared_ptr<RunPrincipal> primaryPrincipal = primaryFileSequence_->readRun_();
      boost::shared_ptr<RunPrincipal> secondaryPrincipal = secondaryFileSequence_->readIt(primaryPrincipal->id());
      if (secondaryPrincipal.get() != 0) {
        checkConsistency(*primaryPrincipal, *secondaryPrincipal);
        primaryPrincipal->recombine(*secondaryPrincipal, branchIDsToReplace_[InRun]);
      } else {
        throw edm::Exception(errors::MismatchedInputFiles, "PoolSource::readRun_")
          << " Run " << primaryPrincipal->run()
          << " is not found in the secondary input files\n";
      }
      return primaryPrincipal;
    }
    return primaryFileSequence_->readRun_();
  }

  boost::shared_ptr<SubRunPrincipal>
  PoolSource::readSubRun_() {
    if (secondaryFileSequence_ && !branchIDsToReplace_[InSubRun].empty()) {
      boost::shared_ptr<SubRunPrincipal> primaryPrincipal = primaryFileSequence_->readSubRun_();
      boost::shared_ptr<SubRunPrincipal> secondaryPrincipal = secondaryFileSequence_->readIt(primaryPrincipal->id());
      if (secondaryPrincipal.get() != 0) {
        checkConsistency(*primaryPrincipal, *secondaryPrincipal);
        primaryPrincipal->recombine(*secondaryPrincipal, branchIDsToReplace_[InSubRun]);
      } else {
        throw edm::Exception(errors::MismatchedInputFiles, "PoolSource::readSubRun_")
          << " Run " << primaryPrincipal->run()
          << " SubRun " << primaryPrincipal->subRun()
          << " is not found in the secondary input files\n";
      }
      return primaryPrincipal;
    }
    return primaryFileSequence_->readSubRun_();
  }

  std::auto_ptr<EventPrincipal>
  PoolSource::readEvent_() {
    if (secondaryFileSequence_ && !branchIDsToReplace_[InEvent].empty()) {
      std::auto_ptr<EventPrincipal> primaryPrincipal = primaryFileSequence_->readEvent_();
      std::auto_ptr<EventPrincipal> secondaryPrincipal = secondaryFileSequence_->readIt(primaryPrincipal->id(), primaryPrincipal->subRun(), true);
      if (secondaryPrincipal.get() != 0) {
        checkConsistency(*primaryPrincipal, *secondaryPrincipal);
        primaryPrincipal->recombine(*secondaryPrincipal, branchIDsToReplace_[InEvent]);
      } else {
        throw edm::Exception(errors::MismatchedInputFiles, "PoolSource::readEvent_") <<
          primaryPrincipal->id() << " is not found in the secondary input files\n";
      }
      return primaryPrincipal;
    }
    EventSourceSentry(*this);
    return primaryFileSequence_->readEvent_();
  }

  std::auto_ptr<EventPrincipal>
  PoolSource::readIt(EventID const& id) {
    if (secondaryFileSequence_) {
      std::auto_ptr<EventPrincipal> primaryPrincipal = primaryFileSequence_->readIt(id);
      std::auto_ptr<EventPrincipal> secondaryPrincipal = secondaryFileSequence_->readIt(id, primaryPrincipal->subRun(), true);
      if (secondaryPrincipal.get() != 0) {
        checkConsistency(*primaryPrincipal, *secondaryPrincipal);
        primaryPrincipal->recombine(*secondaryPrincipal, branchIDsToReplace_[InEvent]);
      } else {
        throw edm::Exception(errors::MismatchedInputFiles, "PoolSource::readIt") <<
          primaryPrincipal->id() << " is not found in the secondary input files\n";
      }
      return primaryPrincipal;
    }
    EventSourceSentry(*this);
    return primaryFileSequence_->readIt(id);
  }

  InputSource::ItemType
  PoolSource::getNextItemType() {
    return primaryFileSequence_->getNextItemType();
  }

  // Rewind to before the first event that was read.
  void
  PoolSource::rewind_() {
    primaryFileSequence_->rewind_();
  }

  // Advance "offset" events.  Offset can be positive or negative (or zero).
  void
  PoolSource::skip(int offset) {
    primaryFileSequence_->skip(offset);
  }

  void
  PoolSource::readMany_(int number, EventPrincipalVector& result) {
    assert (!secondaryFileSequence_);
    primaryFileSequence_->readMany_(number, result);
  }

  void
  PoolSource::readMany_(int number, EventPrincipalVector& result, EventID const& id, unsigned int fileSeqNumber) {
    assert (!secondaryFileSequence_);
    primaryFileSequence_->readMany_(number, result, id, fileSeqNumber);
  }

  void
  PoolSource::readManyRandom_(int number, EventPrincipalVector& result, unsigned int& fileSeqNumber) {
    assert (!secondaryFileSequence_);
    primaryFileSequence_->readManyRandom_(number, result, fileSeqNumber);
  }

  void
  PoolSource::dropUnwantedBranches_(std::vector<std::string> const& wantedBranches) {
    assert (!secondaryFileSequence_);
    assert (!primary());
    primaryFileSequence_->dropUnwantedBranches_(wantedBranches);
  }

  void
  PoolSource::fillDescription(fhicl::ParameterSetDescription& iDesc,
                              std::string const& moduleLabel) {

    iDesc.addOptionalUntracked<unsigned int>("firstRun", 1U);
    iDesc.addOptionalUntracked<unsigned int>("firstSubRun", 1U);
    iDesc.addOptionalUntracked<unsigned int>("firstEvent", 1U);
    iDesc.addOptionalUntracked<unsigned int>("skipEvents", 0U);

    std::vector<SubRunID> defaultSubRuns;
    iDesc.addOptionalUntracked<std::vector<SubRunID> >("subRunsToSkip", defaultSubRuns);

    std::vector<EventID> defaultEvents;
    iDesc.addOptionalUntracked<std::vector<EventID> >("eventsToProcess", defaultEvents);

    iDesc.addOptionalUntracked<bool>("noEventSort", false);
    iDesc.addOptionalUntracked<bool>("skipBadFiles", false);
    iDesc.addOptionalUntracked<bool>("dropDescendantsOfDroppedBranches", true);
    iDesc.addOptionalUntracked<unsigned int>("cacheSize", 0U);
    iDesc.addOptionalUntracked<int>("treeMaxVirtualSize", -1);
    iDesc.addOptionalUntracked<unsigned int>("setRunNumber", 0U);

    std::vector<std::string> defaultStrings(1U, std::string("keep *"));
    iDesc.addOptionalUntracked<std::vector<std::string> >("inputCommands", defaultStrings);

    std::string defaultString("permissive");
    iDesc.addOptionalUntracked<std::string>("fileMatchMode", defaultString);

    defaultString = "checkEachRealDataFile";
    iDesc.addOptionalUntracked<std::string>("duplicateCheckMode", defaultString);

    defaultStrings.clear();
    iDesc.addUntracked<std::vector<std::string> >("fileNames", defaultStrings);
    iDesc.addOptionalUntracked<std::vector<std::string> >("secondaryFileNames", defaultStrings);

    defaultString.clear();
    iDesc.addOptionalUntracked<std::string>("overrideCatalog", defaultString);

    defaultString = "RunsSubRunsAndEvents";
    iDesc.addOptionalUntracked<std::string>("processingMode", defaultString);
  }

}  // namespace edm
