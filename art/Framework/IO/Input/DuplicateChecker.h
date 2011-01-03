#ifndef DataFormats_Provenance_DuplicateChecker_h
#define DataFormats_Provenance_DuplicateChecker_h

// ======================================================================
//
// DuplicateChecker - Used by PoolInput to detect events with the same
//                    run and event number.
//
// The class can be configured to check for duplicates
//   - within the scope of each single input file, or
//   - all input files, or
//   - not at all.
//
// ======================================================================

#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "fhiclcpp/ParameterSet.h"
#include <set>
#include <string>

// ----------------------------------------------------------------------

namespace art {

  class FileIndex;

  class DuplicateChecker {
  public:

    DuplicateChecker(fhicl::ParameterSet const& pset);

    void init(bool realData,
              FileIndex const& fileIndex);

    void inputFileClosed();

    void rewind();

    bool isDuplicateAndCheckActive(EventID const& eventID,
                                   SubRunNumber_t const& subRun,
                                   std::string const& fileName);

  private:

    enum DuplicateCheckMode { noDuplicateCheck, checkEachFile, checkEachRealDataFile, checkAllFilesOpened };

    DuplicateCheckMode duplicateCheckMode_;

    enum DataType { isRealData, isSimulation, unknown };

    DataType dataType_;

    std::set<EventID> eventIDs_;

    bool itIsKnownTheFileHasNoDuplicates_;
  };  // DuplicateChecker

}  // art

// ======================================================================

#endif
