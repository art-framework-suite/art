#ifndef art_Framework_Services_Registry_detail_verifyContext_h
#define art_Framework_Services_Registry_detail_verifyContext_h
#include "art/Persistency/Provenance/ExecEnvInfo.h"

namespace art {
  namespace detail {
    bool verifyContext(ExecEnvInfo const & oc);
  }
}

#endif /* art_Framework_Services_Registry_detail_verifyContext_h */

// Local Variables:
// mode: c++
// End:
