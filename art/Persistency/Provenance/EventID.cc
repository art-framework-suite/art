#include "art/Persistency/Provenance/EventID.h"
#include "art/Utilities/Exception.h"

#include <ostream>

inline
art::EventID
art::EventID::
flushEvent(RunID rID, SubRunID srID)
{
  if (rID != srID.runID()) {
    throw Exception(errors::LogicError)
      << "Inconsistency between provided rID and srID.runID().";
  }
  return EventID(srID, FlushFlag());
}

std::ostream &
art::operator<<(std::ostream & os, EventID const & iID)
{
  os << iID.subRun_ << " event: ";
  if (iID.isFlush()) {
    os << "FLUSH";
  }
  else if (iID.isValid()) {
    os << iID.event_;
  }
  else {
    os << "INVALID";
  }
  return os;
}
