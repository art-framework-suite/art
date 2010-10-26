#include "art/Persistency/Provenance/RunID.h"
#include <ostream>

namespace art {
  std::ostream& operator<<(std::ostream& oStream, RunID const& iID) {
    oStream << "run: " << iID.run();
    return oStream;
  }
}
