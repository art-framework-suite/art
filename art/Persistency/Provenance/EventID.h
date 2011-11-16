#ifndef art_Persistency_Provenance_EventID_h
#define art_Persistency_Provenance_EventID_h

// An EventID labels an unique readout of the data acquisition system,
// which we call an "event".

#include "art/Persistency/Provenance/SortInvalidFirst.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "cpp0x/cstdint"
#include <ostream>

namespace art {
   typedef std::uint32_t EventNumber_t;
   class EventID;
}

class art::EventID {
public:
   EventID() : subRun_(), event_() {}
   EventID(RunNumber_t r, SubRunNumber_t sr, EventNumber_t e) :
      subRun_(r, sr), event_(inRangeOrInvalid(e))
   {}
   // This needs to be done in enough places in the framework that this
   // constructor should be public.
   EventID(SubRunID const &sID, EventNumber_t e) :
      subRun_(sID), event_(inRangeOrInvalid(e))
   {}


   bool isValid() const {
      return (event_ != INVALID_EVENT_NUMBER) && subRun_.isValid();
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

   EventID nextSubRun(EventNumber_t first_event = FIRST_EVENT_NUMBER) const {
      return EventID(subRun_.next(), first_event);
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
      static SortInvalidFirst<EventNumber_t> op(INVALID_EVENT_NUMBER);
      if (subRun_ == other.subRun_) {
         return op(event_, other.event_);
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
      oStream << iID.subRun_ << " event: ";
      if (iID.event_ == INVALID_EVENT_NUMBER) {
        oStream << "INVALID";
      } else {
        oStream << iID.event_;
      }
      return oStream;
   }

private:
   static EventNumber_t const INVALID_EVENT_NUMBER;
   static EventNumber_t const MAX_VALID_EVENT_NUMBER;
   static EventNumber_t const FIRST_EVENT_NUMBER;

   EventNumber_t inRangeOrInvalid(EventNumber_t e) {
      return (e < FIRST_EVENT_NUMBER ||
              e > MAX_VALID_EVENT_NUMBER)?INVALID_EVENT_NUMBER:e;
   }

   SubRunID subRun_;
   EventNumber_t event_;
};

#endif /* art_Persistency_Provenance_EventID_h */

// Local Variables:
// mode: c++
// End:
