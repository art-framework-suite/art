#ifndef art_Framework_Modules_SampledEventID_h
#define art_Framework_Modules_SampledEventID_h

#include "canvas/Persistency/Provenance/EventID.h"

#include <string>

namespace art {
  struct SampledEventID {
    EventID id;
    std::string dataset;
    double weight;
    double probability;
  };

  inline std::ostream&
  operator<<(std::ostream& os, SampledEventID const& sampledID)
  {
    os << "Sampled EventID: '" << sampledID.id << "'  Dataset: '"
       << sampledID.dataset << "'  Weight: " << sampledID.weight
       << "  Probability: " << sampledID.probability;
    return os;
  }
}

#endif /* art_Framework_Modules_SampledEventID_h */

// Local Variables:
// mode: c++
// End:
