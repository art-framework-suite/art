#include "canvas/Persistency/Provenance/RunID.h"

#include <ostream>

std::ostream &
art::operator<<(std::ostream & os, art::RunID const & iID)
{
  os << "run: ";
  if (iID.isFlush()) {
    os << "FLUSH";
  }
  else if (iID.isValid()) {
    os << iID.run_;
  }
  else {
    os << "INVALID";
  }
  return os;
}
