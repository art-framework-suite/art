#ifndef art_Persistency_Provenance_SubRunID_h
#define art_Persistency_Provenance_SubRunID_h

// A SubRunID represents a unique period within a run.

#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SortInvalidFirst.h"
#include "art/Utilities/Exception.h"
#include "cpp0x/cstdint"
#include <ostream>

namespace art {
  typedef std::uint32_t SubRunNumber_t;
  class SubRunID;

  std::ostream &
  operator<<(std::ostream & os,
             SubRunID const & iID);
}

class art::SubRunID {
public:
  SubRunID();
  SubRunID(RunID rID, SubRunNumber_t srID);
  SubRunID(RunNumber_t rID,
           SubRunNumber_t srID);

  RunID const & runID() const;
  RunNumber_t run() const;
  SubRunNumber_t subRun() const;

  bool isValid() const;
  bool isFlush() const;

  SubRunID next() const;
  SubRunID nextRun() const;
  SubRunID previous() const;
  SubRunID previousRun() const;

  static SubRunID maxSubRun();
  static SubRunID firstSubRun();
  static SubRunID firstSubRun(RunID const & rID);
  static SubRunID invalidSubRun(RunID const & rID);
  static SubRunID flushSubRun();
  static SubRunID flushSubRun(RunID const & rID);

  // Comparison operators.
  bool operator==(SubRunID const & other) const;
  bool operator!=(SubRunID const & other) const;
  bool operator<(SubRunID const & other) const;
  bool operator<=(SubRunID const & other) const;
  bool operator>(SubRunID const & other) const;
  bool operator>=(SubRunID const & other) const;

  friend std::ostream &
  operator<<(std::ostream & os, SubRunID const & iID);

private:
#ifndef __GCCXML__
  struct FlushFlag { };

  explicit SubRunID(FlushFlag);
  SubRunID(RunID rID, FlushFlag);

  SubRunNumber_t inRangeOrInvalid(SubRunNumber_t sr);

  void checkSane();

  static constexpr SubRunNumber_t INVALID_SUBRUN_NUMBER();
  static constexpr SubRunNumber_t MAX_VALID_SUBRUN_NUMBER();
  static constexpr SubRunNumber_t FLUSH_SUBRUN_NUMBER();
  static constexpr SubRunNumber_t MAX_NATURAL_SUBRUN_NUMBER();
  static constexpr SubRunNumber_t FIRST_SUBRUN_NUMBER();
#endif

  RunID run_;
  SubRunNumber_t subRun_;
};

#ifndef __GCCXML__
inline
art::SubRunID::
SubRunID()
  :
  run_(),
  subRun_(INVALID_SUBRUN_NUMBER())
{
}

inline
art::SubRunID::
SubRunID(RunNumber_t rID, SubRunNumber_t srID)
  :
  run_(rID),
  subRun_(inRangeOrInvalid(srID))
{
  checkSane();
}

inline
art::RunID const &
art::SubRunID::
runID() const
{
  return run_;
}

inline
art::RunNumber_t
art::SubRunID::
run() const
{
  return run_.run();
}

inline
art::SubRunNumber_t
art::SubRunID::subRun() const
{
  return subRun_;
}

inline
bool
art::SubRunID::
isValid() const
{
  return (subRun_ != INVALID_SUBRUN_NUMBER() && run_.isValid());
}

inline
bool
art::SubRunID::
isFlush() const
{
  return (subRun_ == FLUSH_SUBRUN_NUMBER());
}

inline
art::SubRunID
art::SubRunID::
next() const
{
  if (!isValid()) {
    throw art::Exception(art::errors::InvalidNumber)
        << "cannot increment invalid subrun number.";
  }
  else if (subRun_ == MAX_NATURAL_SUBRUN_NUMBER()) {
    return nextRun();
  }
  return SubRunID(run_, subRun_ + 1);
}

inline
art::SubRunID
art::SubRunID::
nextRun() const
{
  return SubRunID(run_.next(), FIRST_SUBRUN_NUMBER());
}

inline
art::SubRunID
art::SubRunID::
previous() const
{
  if (!isValid()) {
    throw art::Exception(art::errors::InvalidNumber)
        << "cannot decrement invalid subrun number.";
  }
  else if (subRun_ == FIRST_SUBRUN_NUMBER()) {
    return previousRun();
  }
  return SubRunID(run_, subRun_ - 1);
}

inline
art::SubRunID
art::SubRunID::
previousRun() const
{
  return SubRunID(run_.previous(), MAX_NATURAL_SUBRUN_NUMBER());
}

inline
art::SubRunID
art::SubRunID::
maxSubRun()
{
  return SubRunID(RunID::maxRun(), MAX_NATURAL_SUBRUN_NUMBER());
}

inline
art::SubRunID
art::SubRunID::
firstSubRun()
{
  return SubRunID(RunID::firstRun(), FIRST_SUBRUN_NUMBER());
}

inline
art::SubRunID
art::SubRunID::
firstSubRun(RunID const & rID)
{
  return SubRunID(rID, FIRST_SUBRUN_NUMBER());
}

inline
art::SubRunID
art::SubRunID::
invalidSubRun(RunID const & rID)
{
  return SubRunID(rID, INVALID_SUBRUN_NUMBER());
}

inline
art::SubRunID
art::SubRunID::
flushSubRun()
{
  return SubRunID(FlushFlag());
}

inline
art::SubRunID
art::SubRunID::
flushSubRun(RunID const & rID)
{
  return SubRunID(rID, FlushFlag());
}

// Comparison operators.
inline
bool
art::SubRunID::
operator==(SubRunID const & other) const
{
  return other.run_ == run_ && other.subRun_ == subRun_;
}

inline
bool
art::SubRunID::
operator!=(SubRunID const & other) const
{
  return !(*this == other);
}

inline
bool
art::SubRunID::
operator<(SubRunID const & other) const
{
  static SortInvalidFirst<SubRunNumber_t> op(INVALID_SUBRUN_NUMBER());
  if (run_ == other.run_) {
    return op(subRun_, other.subRun_);
  }
  else {
    return run_ < other.run_;
  }
}

inline
bool
art::SubRunID::
operator<=(SubRunID const & other) const
{
  return (*this < other) || (*this == other);
}

inline
bool
art::SubRunID::
operator>(SubRunID const & other) const
{
  return (other < *this);
}

inline
bool
art::SubRunID::
operator>=(SubRunID const & other) const
{
  return !(*this < other);
}

inline
art::SubRunID::
SubRunID(FlushFlag)
  :
  run_(RunID::flushRun()),
  subRun_(FLUSH_SUBRUN_NUMBER())
{
}

inline
art::SubRunID::
SubRunID(RunID rID, FlushFlag)
  :
  run_(std::move(rID)),
  subRun_(FLUSH_SUBRUN_NUMBER())
{
}

inline
art::SubRunID::
SubRunID(RunID rID, SubRunNumber_t srID) :
  run_(std::move(rID)), subRun_(inRangeOrInvalid(srID))
{
  checkSane();
}

inline
art::SubRunNumber_t
art::SubRunID::
inRangeOrInvalid(SubRunNumber_t sr)
{
  if (sr == INVALID_SUBRUN_NUMBER() ||
      (sr >= FIRST_SUBRUN_NUMBER() &&
       sr <= MAX_NATURAL_SUBRUN_NUMBER())) {
    return sr;
  }
  else {
    throw Exception(errors::InvalidNumber)
      << "Attempt to construct SubRunID with an invalid number.\n"
      << "Maybe you want SubRunID::flushSubRun()?";
  }
}

inline
void
art::SubRunID::
checkSane()
{
  if (isValid() && ! run_.isValid()) {
    throw art::Exception(art::errors::InvalidNumber)
        << "SubRunID is not meaningful with valid subRun and invalid Run.";
  }
}

constexpr
art::SubRunNumber_t
art::SubRunID::
INVALID_SUBRUN_NUMBER()
{
  return -1;
}

constexpr
art::SubRunNumber_t
art::SubRunID::
MAX_VALID_SUBRUN_NUMBER()
{
  return INVALID_SUBRUN_NUMBER() - 1;
}

constexpr
art::SubRunNumber_t
art::SubRunID::
FLUSH_SUBRUN_NUMBER()
{
  return MAX_VALID_SUBRUN_NUMBER();
}

constexpr
art::SubRunNumber_t
art::SubRunID::
MAX_NATURAL_SUBRUN_NUMBER()
{
  return FLUSH_SUBRUN_NUMBER() - 1;
}

constexpr
art::SubRunNumber_t
art::SubRunID::
FIRST_SUBRUN_NUMBER()
{
  return 0;
}
#endif /* __GCCXML__ */

#endif /* art_Persistency_Provenance_SubRunID_h */

// Local Variables:
// mode: c++
// End:
