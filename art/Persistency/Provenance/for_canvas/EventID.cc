#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Utilities/Exception.h"

#include <ostream>

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
