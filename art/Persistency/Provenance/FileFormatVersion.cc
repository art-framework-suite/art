#include "art/Persistency/Provenance/FileFormatVersion.h"
#include <ostream>

namespace art {
  std::ostream&
  operator<< (std::ostream& os, FileFormatVersion const& ff) {
    os << ff.value_;
    return os;
  }
}

