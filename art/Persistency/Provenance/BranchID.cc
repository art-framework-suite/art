#include "art/Persistency/Provenance/BranchID.h"
#include "cetlib/crc32.h"
#include <ostream>

namespace art {

  BranchID::value_type
  BranchID::toID(std::string const& branchName) {
    cet::crc32 c(branchName);
    return c.digest();
  }

  std::ostream&
  operator<<(std::ostream& os, BranchID const& id) {
    os << id.id();
    return os;
  }
}
