#ifndef art_Persistency_Provenance_RunID_h
#define art_Persistency_Provenance_RunID_h

//
// A RunID represents a unique period of operation of the data
// acquisition system, which we call a "run".
//
// Each RunID contains a fixed-size unsigned integer, the run number.
//

#include "art/Persistency/Provenance/SortInvalidFirst.h"
#include "art/Utilities/Exception.h"
#include "cpp0x/cstdint"
#include <ostream>

namespace art {
  typedef std::uint32_t RunNumber_t;
  class RunID;
}

class art::RunID {
public:
  RunID() : run_(INVALID_RUN_NUMBER) {}
  explicit RunID(RunNumber_t i) :
    run_(inRangeOrInvalid(i)) {}

  RunNumber_t run() const { return run_; }

  bool isValid() const {
    return (run_ != INVALID_RUN_NUMBER);
  }

  RunID next() const {
    if (!isValid()) {
      throw art::Exception(art::errors::InvalidNumber)
          << "cannot increment invalid run number.";
    }
    else if (run_ == MAX_VALID_RUN_NUMBER) {
      throw art::Exception(art::errors::InvalidNumber)
          << "cannot increment maximum run number.";
    }
    return RunID(run_ + 1);
  }

  RunID previous() const {
    if (!isValid()) {
      throw art::Exception(art::errors::InvalidNumber)
          << "cannot decrement minimum run number.";
    }
    else if (run_ == MAX_VALID_RUN_NUMBER) {
      throw art::Exception(art::errors::InvalidNumber)
          << "cannot increment maximum run number.";
    }
    return RunID(run_ - 1);
  }

  static RunID maxRun() {
    return RunID(MAX_VALID_RUN_NUMBER);
  }

  static RunID firstRun() {
    return RunID(FIRST_RUN_NUMBER);
  }

  // Valid comparison operators
  bool operator==(RunID const & other) const {
    return other.run_ == run_;
  }

  bool operator!=(RunID const & other) const {
    return !(*this == other);
  }

  bool operator<(RunID const & other) const {
    static SortInvalidFirst<RunNumber_t> op(INVALID_RUN_NUMBER);
    return op(run_, other.run_);
  }

  bool operator<=(RunID const & other) const {
    return (*this < other) || (*this == other);
  }

  bool operator>(RunID const & other) const {
    return (other < *this);
  }

  bool operator>=(RunID const & other) const {
    return !(*this < other);
  }

  friend inline std::ostream &
  operator<<(std::ostream & oStream, RunID const & iID) {
    oStream << "run: ";
    if (iID.isValid()) {
      oStream << iID.run_;
    }
    else {
      oStream << "INVALID";
    }
    return oStream;
  }

private:
  static RunNumber_t const INVALID_RUN_NUMBER;
  static RunNumber_t const MAX_VALID_RUN_NUMBER;
  static RunNumber_t const FIRST_RUN_NUMBER;

  RunNumber_t inRangeOrInvalid(RunNumber_t r) {
    return (r < FIRST_RUN_NUMBER ||
            r > MAX_VALID_RUN_NUMBER) ? INVALID_RUN_NUMBER : r;
  }

  RunNumber_t run_;
};

#endif /* art_Persistency_Provenance_RunID_h */

// Local Variables:
// mode: c++
// End:
