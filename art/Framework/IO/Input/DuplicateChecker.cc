
#include "art/Framework/IO/Input/DuplicateChecker.h"
#include "art/ParameterSet/ParameterSet.h"
#include "art/Utilities/Exception.h"
#include "MessageFacility/MessageLogger.h"
#include "art/Persistency/Provenance/FileIndex.h"

#include <cassert>

namespace edm {

  DuplicateChecker::DuplicateChecker(ParameterSet const& pset) :

    duplicateCheckMode_(checkEachRealDataFile),
    dataType_(unknown),
    eventIDs_(),
    itIsKnownTheFileHasNoDuplicates_(false)
  {
    std::string duplicateCheckMode =
      pset.getUntrackedParameter<std::string>("duplicateCheckMode", std::string("checkEachRealDataFile"));

    if (duplicateCheckMode == std::string("noDuplicateCheck")) duplicateCheckMode_ = noDuplicateCheck;
    else if (duplicateCheckMode == std::string("checkEachFile")) duplicateCheckMode_ = checkEachFile;
    else if (duplicateCheckMode == std::string("checkEachRealDataFile")) duplicateCheckMode_ = checkEachRealDataFile;
    else if (duplicateCheckMode == std::string("checkAllFilesOpened")) duplicateCheckMode_ = checkAllFilesOpened;
    else {
      throw cms::Exception("Configuration")
        << "Illegal configuration parameter value passed to PoolSource for\n"
        << "the \"duplicateCheckMode\" parameter, legal values are:\n"
        << "\"noDuplicateCheck\", \"checkEachFile\", \"checkEachRealDataFile\", \"checkAllFilesOpened\"\n";
    }
  }

  void DuplicateChecker::init(bool realData,
                              FileIndex const& fileIndex)
  {
    if (duplicateCheckMode_ == noDuplicateCheck) return;
    if (duplicateCheckMode_ == checkAllFilesOpened) return;

    assert(dataType_ == unknown);
    dataType_ = realData ? isRealData : isSimulation;

    if (duplicateCheckMode_ == checkEachFile ||
        (duplicateCheckMode_ == checkEachRealDataFile && dataType_ == isRealData)) {
      itIsKnownTheFileHasNoDuplicates_ = fileIndex.eventsUniqueAndOrdered();
    }
  }

  void DuplicateChecker::inputFileClosed()
  {
    if (duplicateCheckMode_ == noDuplicateCheck) return;
    if (duplicateCheckMode_ == checkAllFilesOpened) return;

    dataType_ = unknown;
    eventIDs_.clear();
    itIsKnownTheFileHasNoDuplicates_ = false;
  }

  void DuplicateChecker::rewind()
  {
    eventIDs_.clear();
  }

  bool DuplicateChecker::isDuplicateAndCheckActive(EventID const& eventID,
                                                   SubRunNumber_t const& subRun,
                                                   std::string const& fileName)
  {
    if (duplicateCheckMode_ == noDuplicateCheck) return false;
    if (duplicateCheckMode_ == checkEachRealDataFile && dataType_ == isSimulation) return false;

    if (duplicateCheckMode_ == checkEachFile ||
        duplicateCheckMode_ == checkEachRealDataFile) {
      assert(dataType_ != unknown);
      if (itIsKnownTheFileHasNoDuplicates_) return false;
    }

    bool duplicate = !eventIDs_.insert(eventID).second;

    if (duplicate) {
      if (duplicateCheckMode_ == checkAllFilesOpened) {
        LogWarning("DuplicateEvent")
          << "Duplicate Events found in entire set of input files.\n"
          << "Both events were from run " << eventID.run()
          << " with event number " << eventID.event() << ".\n"
          << "The duplicate had subRun number " << subRun
          << " and was from file " << fileName << ".\n"
          << "The duplicate will be skipped.\n";
      }
      else {
        LogWarning("DuplicateEvent")
          << "Duplicate Events found in file " << fileName << ".\n"
          << "Both events were from run " << eventID.run()
          << " with event number " << eventID.event() << ".\n"
          << "The duplicate had subRun number " << subRun << ".\n"
          << "The duplicate will be skipped.\n";
      }
      return true;
    }
    return false;
  }
}
