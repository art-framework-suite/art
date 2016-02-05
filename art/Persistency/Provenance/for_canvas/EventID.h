#ifndef art_Persistency_Provenance_EventID_h
#define art_Persistency_Provenance_EventID_h

// An EventID labels an unique readout of the data acquisition system,
// which we call an "event".

#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Utilities/Exception.h"

#include <cstdint>
#include <iosfwd>

namespace art {
  typedef std::uint32_t EventNumber_t;
  class EventID;

  std::ostream &
  operator<<(std::ostream & os, EventID const & iID);
}

class art::EventID {
public:
  EventID();
  EventID(RunNumber_t r, SubRunNumber_t sr, EventNumber_t e);

  // This needs to be done in enough places in the framework that this
  // constructor should be public.
  EventID(SubRunID const & sID, EventNumber_t e);

  RunID const & runID() const;
  RunNumber_t run() const;
  SubRunID const & subRunID() const;
  SubRunNumber_t subRun() const;
  EventNumber_t event() const;

  bool isValid() const;
  bool isFlush() const;

  EventID next() const;
  EventID nextSubRun(EventNumber_t first = FIRST_EVENT_NUMBER()) const;
  EventID nextRun() const;
  EventID previous() const;
  EventID previousSubRun() const;
  EventID previousRun() const;

  static EventID maxEvent();
  static EventID firstEvent();
  static EventID firstEvent(SubRunID const & srID);
  static EventID invalidEvent();
  static EventID invalidEvent(RunID rID);
  static EventID invalidEvent(SubRunID const & srID);
  static EventID flushEvent();
  static EventID flushEvent(RunID rID);
  static EventID flushEvent(SubRunID srID);

  // Comparison operators.
  bool operator==(EventID const & other) const;
  bool operator!=(EventID const & other) const;
  bool operator<(EventID const & other) const;
  bool operator>(EventID const & other) const;
  bool operator<=(EventID const & other) const;
  bool operator>=(EventID const & other) const;

  friend std::ostream &
  operator<<(std::ostream & os, EventID const & iID);

private:
  struct FlushFlag { };

  explicit EventID(FlushFlag);
  EventID(RunID rID, FlushFlag);
  EventID(SubRunID srID, FlushFlag);

  EventNumber_t inRangeOrInvalid(EventNumber_t e);

  static constexpr EventNumber_t INVALID_EVENT_NUMBER();
  static constexpr EventNumber_t MAX_VALID_EVENT_NUMBER();
  static constexpr EventNumber_t FLUSH_EVENT_NUMBER();
  static constexpr EventNumber_t MAX_NATURAL_EVENT_NUMBER();
  static constexpr EventNumber_t FIRST_EVENT_NUMBER();

  SubRunID subRun_;
  EventNumber_t event_;
};

#ifndef __GCCXML__
inline
art::EventID::
EventID()
  :
  subRun_(),
  event_()
{
}

inline
art::EventID::
EventID(RunNumber_t r, SubRunNumber_t sr, EventNumber_t e)
  :
  subRun_(r, sr),
  event_(inRangeOrInvalid(e))
{
}

inline
art::EventID::
EventID(SubRunID const & sID, EventNumber_t e)
  :
  subRun_(sID),
  event_(inRangeOrInvalid(e))
{
}

inline
art::RunID const &
art::EventID::
runID() const
{
  return subRun_.runID();
}

inline
art::RunNumber_t
art::EventID::
run() const
{
  return subRun_.run();
}

inline
art::SubRunID const &
art::EventID::
subRunID() const
{
  return subRun_;
}

inline
art::SubRunNumber_t
art::EventID::
subRun() const
{
  return subRun_.subRun();
}

inline
art::EventNumber_t
art::EventID::
event() const
{
  return event_;
}

inline
bool
art::EventID::
isValid() const
{
  return (event_ != INVALID_EVENT_NUMBER()) && subRun_.isValid();
}

inline
bool
art::EventID::
isFlush() const
{
  return (event_ == FLUSH_EVENT_NUMBER());
}

inline
art::EventID
art::EventID::next() const
{
  if (!isValid()) {
    throw art::Exception(art::errors::InvalidNumber)
        << "cannot increment invalid event number.";
  }
  else if (event_ == MAX_NATURAL_EVENT_NUMBER()) {
    return nextSubRun();
  }
  else {
    return EventID(subRun_, event_ + 1u);
  }
}

inline
art::EventID
art::EventID::
nextSubRun(EventNumber_t first) const
{
  return EventID(subRun_.next(), first);
}

inline
art::EventID
art::EventID::
nextRun() const
{
  return EventID(subRun_.nextRun(), FIRST_EVENT_NUMBER());
}

inline
art::EventID
art::EventID::
previous() const
{
  if (!isValid()) {
    throw art::Exception(art::errors::InvalidNumber)
        << "cannot decrement invalid event number.";
  }
  else if (event_ == FIRST_EVENT_NUMBER()) {
    return previousSubRun();
  }
  else {
    return EventID(subRun_, event_ - 1u);
  }
}

inline
art::EventID
art::EventID::
previousSubRun() const
{
  return EventID(subRun_.previous(), MAX_NATURAL_EVENT_NUMBER());
}

inline
art::EventID
art::EventID::
previousRun() const
{
  return EventID(subRun_.previousRun(), MAX_NATURAL_EVENT_NUMBER());
}

inline
art::EventID
art::EventID::
maxEvent()
{
  return EventID(SubRunID::maxSubRun(), MAX_NATURAL_EVENT_NUMBER());
}

inline
art::EventID
art::EventID::
firstEvent()
{
  return EventID(SubRunID::firstSubRun(), FIRST_EVENT_NUMBER());
}

inline
art::EventID
art::EventID::
firstEvent(SubRunID const & srID)
{
  return EventID(srID, FIRST_EVENT_NUMBER());
}

inline
art::EventID
art::EventID::
invalidEvent()
{
  return EventID();
}

inline
art::EventID
art::EventID::
invalidEvent(RunID rID)
{
  return EventID(SubRunID::invalidSubRun(rID), INVALID_EVENT_NUMBER());
}

inline
art::EventID
art::EventID::
invalidEvent(SubRunID const & srID)
{
  return EventID(srID, INVALID_EVENT_NUMBER());
}

inline
art::EventID
art::EventID::
flushEvent()
{
  return EventID(FlushFlag());
}

inline
art::EventID
art::EventID::
flushEvent(RunID rID)
{
  return EventID(rID, FlushFlag());
}

inline
art::EventID
art::EventID::
flushEvent(SubRunID srID)
{
  return EventID(srID, FlushFlag());
}

// Comparison operators.
inline
bool
art::EventID::
operator==(EventID const & other) const
{
  return other.subRun_ == subRun_ && other.event_ == event_;
}

inline
bool
art::EventID::
operator!=(EventID const & other) const
{
  return !(*this == other);
}

#include "art/Persistency/Provenance/SortInvalidFirst.h"

inline
bool
art::EventID::
operator<(EventID const & other) const
{
  static SortInvalidFirst<EventNumber_t> op(INVALID_EVENT_NUMBER());
  if (subRun_ == other.subRun_) {
    return op(event_, other.event_);
  }
  else {
    return subRun_ < other.subRun_;
  }
}

inline
bool
art::EventID::
operator>(EventID const & other) const
{
  return (other < *this);
}

inline
bool
art::EventID::
operator<=(EventID const & other) const
{
  return (*this < other) || (*this == other);
}

inline
bool
art::EventID::
operator>=(EventID const & other) const
{
  return !(*this < other);
}

inline
art::EventID::
EventID(FlushFlag)
  :
  subRun_(SubRunID::flushSubRun()),
  event_(FLUSH_EVENT_NUMBER())
{
}

inline
art::EventID::
EventID(RunID rID, FlushFlag)
  :
  subRun_(SubRunID::flushSubRun(rID)),
  event_(FLUSH_EVENT_NUMBER())
{
}

inline
art::EventID::
EventID(SubRunID srID, FlushFlag)
  :
  subRun_(std::move(srID)),
  event_(FLUSH_EVENT_NUMBER())
{
}

inline
art::EventNumber_t
art::EventID::
inRangeOrInvalid(EventNumber_t e)
{
  if (e == INVALID_EVENT_NUMBER() ||
      e <= MAX_NATURAL_EVENT_NUMBER()) {
    return e;
  }
  else {
    throw Exception(errors::InvalidNumber)
      << "Attempt to construct an EventID with an invalid number.\n"
      << "Maybe you want EventID::flushEvent()?\n";
  }
}

constexpr
art::EventNumber_t
art::EventID::
INVALID_EVENT_NUMBER()
{
  return -1;
}

constexpr
art::EventNumber_t
art::EventID::
MAX_VALID_EVENT_NUMBER()
{
  return INVALID_EVENT_NUMBER() - 1u;
}

constexpr
art::EventNumber_t
art::EventID::
FLUSH_EVENT_NUMBER()
{
  return MAX_VALID_EVENT_NUMBER();
}

constexpr
art::EventNumber_t
art::EventID::
MAX_NATURAL_EVENT_NUMBER()
{
  return FLUSH_EVENT_NUMBER() - 1u;
}

constexpr
art::EventNumber_t
art::EventID::
FIRST_EVENT_NUMBER()
{
  return 1u;
}
#endif /* __GCCXML__ */

#endif /* art_Persistency_Provenance_EventID_h */

// Local Variables:
// mode: c++
// End:
