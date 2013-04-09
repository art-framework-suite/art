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
#include <iosfwd>

namespace art {
  typedef std::uint32_t RunNumber_t;
  class RunID;

  std::ostream &
  operator<<(std::ostream & os,
             art::RunID const & iID);
}

class art::RunID {
public:
  RunID();
  explicit RunID(RunNumber_t i);

  RunNumber_t run() const;

  bool isValid() const;
  bool isFlush() const;

  RunID next() const;
  RunID previous() const;

  static RunID maxRun();
  static RunID firstRun();
  static RunID flushRun();

  // Comparison operators.
  bool operator==(RunID const & other) const;
  bool operator!=(RunID const & other) const;
  bool operator<(RunID const & other) const;
  bool operator<=(RunID const & other) const;
  bool operator>(RunID const & other) const;
  bool operator>=(RunID const & other) const;

  friend std::ostream &
  operator<<(std::ostream & os, RunID const & iID);

private:
#ifndef __GCCXML__
  struct FlushFlag { };

  explicit RunID(FlushFlag);

  RunNumber_t inRangeOrInvalid(RunNumber_t r);

  static constexpr RunNumber_t INVALID_RUN_NUMBER();
  static constexpr RunNumber_t MAX_VALID_RUN_NUMBER();
  static constexpr RunNumber_t FLUSH_RUN_NUMBER();
  static constexpr RunNumber_t MAX_NATURAL_RUN_NUMBER();
  static constexpr RunNumber_t FIRST_RUN_NUMBER();
#endif

  RunNumber_t run_;
};

#ifndef __GCCXML__
inline
art::RunID::
RunID()
  :
  run_(INVALID_RUN_NUMBER())
{
}

inline
art::RunID::
RunID(RunNumber_t i)
  :
  run_(inRangeOrInvalid(i))
{
}

inline
art::RunNumber_t
art::RunID::
run() const
{
  return run_;
}

inline
bool
art::RunID::
isValid() const
{
  return (run_ != INVALID_RUN_NUMBER());
}

inline
bool
art::RunID::
isFlush() const
{
  return (run_ == FLUSH_RUN_NUMBER());
}

inline
art::RunID
art::RunID::
next() const
{
  if (!isValid()) {
    throw Exception(errors::InvalidNumber)
        << "cannot increment invalid run number.";
  }
  else if (run_ == MAX_NATURAL_RUN_NUMBER()) {
    throw Exception(errors::InvalidNumber)
        << "cannot increment maximum run number.";
  }
  return RunID(run_ + 1);
}

inline
art::RunID
art::RunID::
previous() const
{
  if (!isValid()) {
    throw Exception(errors::InvalidNumber)
        << "cannot decrement minimum run number.";
  }
  else if (run_ == MAX_NATURAL_RUN_NUMBER()) {
    throw Exception(errors::InvalidNumber)
        << "cannot increment maximum run number.";
  }
  return RunID(run_ - 1);
}

inline
art::RunID
art::RunID::
maxRun()
{
  return RunID(MAX_NATURAL_RUN_NUMBER());
}

inline
art::RunID
art::RunID::
firstRun()
{
  return RunID(FIRST_RUN_NUMBER());
}

inline
art::RunID
art::RunID::
flushRun()
{
  return RunID(FlushFlag());
}

// Comparison operators.
inline
bool
art::RunID::
operator==(RunID const & other) const
{
  return other.run_ == run_;
}

inline
bool
art::RunID::
operator!=(RunID const & other) const
{
  return !(*this == other);
}

inline
bool
art::RunID::
operator<(RunID const & other) const
{
  static SortInvalidFirst<RunNumber_t> op(INVALID_RUN_NUMBER());
  return op(run_, other.run_);
}

inline
bool
art::RunID::
operator<=(RunID const & other) const
{
  return (*this < other) || (*this == other);
}

inline
bool
art::RunID::
operator>(RunID const & other) const
{
  return (other < *this);
}

inline
bool
art::RunID::
operator>=(RunID const & other) const
{
  return !(*this < other);
}

inline
art::RunNumber_t
art::RunID::inRangeOrInvalid(RunNumber_t r)
{
  if (r == INVALID_RUN_NUMBER() ||
      (r >= FIRST_RUN_NUMBER() &&
       r <=  MAX_NATURAL_RUN_NUMBER())) {
    return r;
  }
  else {
    throw Exception(errors::InvalidNumber)
      << "Attempt to construct RunID with an invalid number.\n"
      << "Maybe you want RunID::flushRun()?";
  }
}

inline
art::RunID::
RunID(FlushFlag)
  :
  run_(FLUSH_RUN_NUMBER())
{
}

constexpr
art::RunNumber_t
art::RunID::
INVALID_RUN_NUMBER()
{
  return -1;
}

constexpr
art::RunNumber_t
art::RunID::
MAX_VALID_RUN_NUMBER()
{
  return INVALID_RUN_NUMBER() - 1;
}

constexpr
art::RunNumber_t
art::RunID::
FLUSH_RUN_NUMBER()
{
  return MAX_VALID_RUN_NUMBER();
}

constexpr
art::RunNumber_t
art::RunID::
MAX_NATURAL_RUN_NUMBER()
{
  return FLUSH_RUN_NUMBER() - 1;
}

constexpr
art::RunNumber_t
art::RunID::
FIRST_RUN_NUMBER()
{
  return 1;
}
#endif /* __GCCXML__ */

#endif /* art_Persistency_Provenance_RunID_h */

// Local Variables:
// mode: c++
// End:
