#ifndef art_Persistency_Provenance_SubRunID_h
#define art_Persistency_Provenance_SubRunID_h

// A SubRunID represents a unique period within a run.

#include "art/Utilities/Exception.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SortInvalidFirst.h"

#include "cpp0x/cstdint"
#include <functional>
#include <ostream>

namespace art {
   typedef uint32_t SubRunNumber_t;
   class SubRunID;
}

class art::SubRunID {
public:
   SubRunID() : run_(), subRun_(INVALID_SUBRUN_NUMBER) {}
   SubRunID(RunNumber_t iRun, SubRunNumber_t iSubRun) :
      run_(iRun), subRun_(inRangeOrInvalid(iSubRun)) {
      checkSane();
   }

   bool isValid() const {
      return (subRun_ != INVALID_SUBRUN_NUMBER);
   }

   RunID const &runID() const { return run_; }
   RunNumber_t run() const { return run_.run(); }
   SubRunNumber_t subRun() const { return subRun_; }

   SubRunID next() const {
      if (!isValid()) {
         throw art::Exception(art::errors::InvalidNumber)
            << "cannot increment invalid subrun number.";
      } else if (subRun_ == MAX_VALID_SUBRUN_NUMBER) {
         return nextRun();
      }
      return SubRunID(run_, subRun_ + 1);
   }

   SubRunID nextRun() const {
      return SubRunID(run_.next(), FIRST_SUBRUN_NUMBER);
   }

   SubRunID previous() const {
      if (!isValid()) {
         throw art::Exception(art::errors::InvalidNumber)
            << "cannot decrement invalid subrun number.";
      } else if (subRun_ == FIRST_SUBRUN_NUMBER) {
         return previousRun();
      }
      return SubRunID(run_, subRun_ - 1);
   }

   SubRunID previousRun() const {
      return SubRunID(run_.previous(), MAX_VALID_SUBRUN_NUMBER);
   }

   static SubRunID maxSubRun() {
      return SubRunID(RunID::maxRun(), MAX_VALID_SUBRUN_NUMBER);
   }

   static SubRunID firstSubRun() {
      return SubRunID(RunID::firstRun(), FIRST_SUBRUN_NUMBER);
   }

   static SubRunID firstSubRun(RunID const &rID) {
      return SubRunID(rID, FIRST_SUBRUN_NUMBER);
   }

   static SubRunID invalidSubRun(RunID const &rID) {
      return SubRunID(rID, INVALID_SUBRUN_NUMBER);
   }

   // Valid comparison operators
   bool operator==(SubRunID const& other) const {
      return other.run_ == run_ && other.subRun_ == subRun_;
   }

   bool operator!=(SubRunID const& other) const {
      return !(*this == other);
   }

   bool operator<(SubRunID const& other) const {
      static SortInvalidFirst<SubRunNumber_t> op(INVALID_SUBRUN_NUMBER);
      if (run_ == other.run_) {
         return op(subRun_, other.subRun_);
      } else {
         return run_ < other.run_;
      }
   }

   bool operator<=(SubRunID const& other) const {
      return (*this < other) || (*this == other);
   }

   bool operator>(SubRunID const& other) const {
      return (other < *this);
   }

   bool operator>=(SubRunID const& other) const {
      return ! (*this < other);
   }

   friend inline std::ostream&
   operator<<(std::ostream& oStream, SubRunID const& iID) {
      oStream << iID.run_ << " subRun: ";
      if (iID.isValid()) {
         oStream << iID.subRun_;
      } else {
         oStream << "INVALID";
      }
      return oStream;
   }

private:
   static SubRunNumber_t const INVALID_SUBRUN_NUMBER;
   static SubRunNumber_t const MAX_VALID_SUBRUN_NUMBER;
   static SubRunNumber_t const FIRST_SUBRUN_NUMBER;

   SubRunID(RunID iRun, SubRunNumber_t iSubRun) :
      run_(iRun), subRun_(inRangeOrInvalid(iSubRun)) {
      checkSane();
   }

   SubRunNumber_t inRangeOrInvalid(SubRunNumber_t sr) {
      return (sr < FIRST_SUBRUN_NUMBER ||
              sr > MAX_VALID_SUBRUN_NUMBER)?INVALID_SUBRUN_NUMBER:sr;
   }

   void checkSane() {
      if (isValid() && ! run_.isValid()) {
         throw art::Exception(art::errors::InvalidNumber)
            << "SubRunID is not meaningful with valid subRun and invalid Run.";
      }
   }

   RunID run_;
   SubRunNumber_t subRun_;
};

#endif /* art_Persistency_Provenance_SubRunID_h */

// Local Variables:
// mode: c++
// End:
