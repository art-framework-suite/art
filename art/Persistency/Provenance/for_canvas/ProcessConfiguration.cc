#include <sstream>

#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "cetlib/MD5Digest.h"
#include <ostream>

/*----------------------------------------------------------------------



----------------------------------------------------------------------*/

namespace art {



  ProcessConfigurationID
  ProcessConfiguration::id() const
  {
    // This implementation is ripe for optimization.
    std::ostringstream oss;
    oss << *this;
    std::string stringrep = oss.str();
    cet::MD5Digest md5alg(stringrep);
    return ProcessConfigurationID(md5alg.digest().toString());
  }

  bool operator<(ProcessConfiguration const& a, ProcessConfiguration const& b) {
    if (a.processName_ < b.processName_) return true;
    if (b.processName_ < a.processName_) return false;
    if (a.parameterSetID_ < b.parameterSetID_) return true;
    if (b.parameterSetID_ < a.parameterSetID_) return false;
    if (a.releaseVersion_ < b.releaseVersion_) return true;
    if (b.releaseVersion_ < a.releaseVersion_) return false;
    if (a.passID_ < b.passID_) return true;
    return false;
  }

  std::ostream&
  operator<< (std::ostream& os, ProcessConfiguration const& pc) {
    os << pc.processName_ << ' '
       << pc.parameterSetID_ << ' '
       << pc.releaseVersion_ << ' '
       << pc.passID_;
    return os;
  }
}
