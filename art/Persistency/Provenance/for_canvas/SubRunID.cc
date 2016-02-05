#include "canvas/Persistency/Provenance/SubRunID.h"

#include <ostream>

std::ostream &
art::operator<<(std::ostream & os, SubRunID const & iID)
{
  os << iID.run_ << " subRun: ";
  if (iID.isFlush()) {
    os << "FLUSH";
  }
  else if (iID.isValid()) {
    os << iID.subRun_;
  }
  else {
    os << "INVALID";
  }
  return os;
}
