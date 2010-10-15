#include "art/Persistency/Provenance/EventID.h"
#include <ostream>

namespace art {
  std::ostream& operator<<(std::ostream& oStream, EventID const& iID) {
    oStream << "run: " << iID.run() << " event: " << iID.event();
    return oStream;
  }
}
