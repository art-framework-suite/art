#include "canvas/Persistency/Provenance/BranchID.h"
#include "cetlib/crc32.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <ostream>

namespace art {

  BranchID::value_type
  BranchID::toID(std::string const& branchName) {
    cet::crc32 c(branchName);
    const auto & check = c.digest();
    mf::LogDebug("BranchID") << "Product created with branch id: "
                             << "[" << check << "] "
                             << "from branch name: "
                             << "\"" << branchName << "\"";
    return check;
  }

  std::ostream&
  operator<<(std::ostream& os, BranchID const& id) {
    os << id.id();
    return os;
  }
}
