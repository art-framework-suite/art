#ifndef art_Persistency_Provenance_EventID_h
#define art_Persistency_Provenance_EventID_h

// An EventID labels an unique readout of the data acquisition system,
// which we call an "event".

#include "art/Persistency/Provenance/SortInvalidFirst.h"
#include "art/Persistency/Provenance/SubRunID.h"

#include "cpp0x/cstdint"
#include <functional>
#include <ostream>

namespace art {
   typedef std::uint32_t EventNumber_t;
   class EventID;
}

class art::EventID {
public:
   EventID() : subRun_(), event_(0) {}
   EventID(RunNumber_t r, SubRunNumber_t sr, EventNumber_t e) :
      subRun_(r, sr), event_(inRangeOrInvalid(e)) {
      checkSane();
   }

   bool isValid() const {
      // Don't need to chack for the invalid value because it's outside
      // the range by construction.
      return (event_ != INVALID_EVENT_NUMBER);
   }

   SubRunID const &subRunID() const { return subRun_; }
   RunID const &runID() const { return subRun_.runID(); }

   RunNumber_t run() const { return subRun_.run(); }
   SubRunNumber_t subRun() const { return subRun_.subRun(); }
   EventNumber_t event() const { return event_; }

   EventID next() const {
      if (!isValid()) {
         throw art::Exception(art::errors::InvalidNumber)
            << "cannot increment invalid event number.";
      } else if (event_ == MAX_VALID_EVENT_NUMBER) {
         return nextSubRun();
      } else {
         return EventID(subRun_, event_ + 1);
      }
   }

   EventID nextSubRun() const {
      return EventID(subRun_.next(), FIRST_EVENT_NUMBER);
   }

   EventID nextRun() const {
      return EventID(subRun_.nextRun(), FIRST_EVENT_NUMBER);
   }

   EventID previous() {
      if (!isValid()) {
         throw art::Exception(art::errors::InvalidNumber)
            << "cannot decrement invalid event number.";
      } else if (event_ == FIRST_EVENT_NUMBER) {
         return previousSubRun();
      } else {
         return EventID(subRun_, event_ - 1);
      }
   }

   EventID previousSubRun() {
      return EventID(subRun_.previous(), MAX_VALID_EVENT_NUMBER);
   }

   EventID previousRun() {
      return EventID(subRun_.previousRun(), MAX_VALID_EVENT_NUMBER);
   }

   static EventID maxEvent() {
      return EventID(SubRunID::maxSubRun(), MAX_VALID_EVENT_NUMBER);
   }

   static EventID firstEvent() {
      return EventID(SubRunID::firstSubRun(), FIRST_EVENT_NUMBER);
   }

   static EventID firstEvent(SubRunID const & srID) {
      return EventID(srID, FIRST_EVENT_NUMBER);
   }

   static EventID invalidEvent() { return EventID(); }

   static EventID invalidEvent(RunID const & rID) {
      return EventID(SubRunID::invalidSubRun(rID), INVALID_EVENT_NUMBER);
   }

   static EventID invalidEvent(SubRunID const &srID) {
      return EventID(srID, INVALID_EVENT_NUMBER);
   }

   // Valid comparison operators
   bool operator==(EventID const& other) const {
      return other.subRun_ == subRun_ && other.event_ == event_;
   }

   bool operator!=(EventID const& other) const {
      return !(*this == other);
   }

   bool operator<(EventID const& other) const {
      static SortInvalidFirst<EventNumber_t> op_e(INVALID_EVENT_NUMBER);
      if (subRun_ == other.subRun_) {
         return op_e(event_, other.event_);
      } else {
         return subRun_ < other.subRun_;
      }
   }

   bool operator>(EventID const& other) const {
      return (other < *this);
   }

   bool operator<=(EventID const& other) const {
      return (*this < other) || (*this == other);
   }


   bool operator>=(EventID const& other) const {
      return ! (*this < other);
   }

   friend inline std::ostream&
   operator<<(std::ostream& oStream, EventID const& iID) {
      oStream << iID.subRun_ << " event: " << iID.event_;
      return oStream;
   }

private:
   static EventNumber_t const INVALID_EVENT_NUMBER;
   static EventNumber_t const MAX_VALID_EVENT_NUMBER;
   static EventNumber_t const FIRST_EVENT_NUMBER;

   EventID(SubRunID const &sID, EventNumber_t e) :
      subRun_(sID), event_(inRangeOrInvalid(e)) {
      checkSane();
   }

   EventNumber_t inRangeOrInvalid(EventNumber_t e) {
      return (e < FIRST_EVENT_NUMBER ||
              e > MAX_VALID_EVENT_NUMBER)?INVALID_EVENT_NUMBER:e;
   }

   void checkSane() {
      if (isValid() && !subRun_.isValid()) {
         throw art::Exception(art::errors::InvalidNumber)
            << "EventID is not meaningful with valid event and invalid subRun.";
      }
   }

   SubRunID subRun_;
   EventNumber_t event_;
};

#endif /* art_Persistency_Provenance_EventID_h */

// Local Variables:
// mode: c++
// End:
