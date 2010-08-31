#include "art/Persistency/Provenance/BranchID.h"
#include "art/Utilities/CRC32Calculator.h"
#include <ostream>

namespace edm {

  BranchID::value_type
  BranchID::toID(std::string const& branchName) {
    cms::CRC32Calculator crc32(branchName);
    return crc32.checksum();
  }

  std::ostream&
  operator<<(std::ostream& os, BranchID const& id) {
    os << id.id();
    return os;
  }
}
