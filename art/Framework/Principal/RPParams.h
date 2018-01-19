#ifndef art_Framework_Principal_RPParams_h
#define art_Framework_Principal_RPParams_h
////////////////////////////////////////////////////////////////////////
// RPParams
//
// Parameters pertinent to a particular ResultsProducer passed via the
// RPWorker.
////////////////////////////////////////////////////////////////////////

#include "fhiclcpp/ParameterSetID.h"

#include <string>

namespace art {
  struct RPParams;
}

struct art::RPParams {
  fhicl::ParameterSetID rpPSetID;
  std::string rpPluginType;
  std::string rpLabel;
};

#endif /* art_Framework_Principal_RPParams_h */

// Local Variables:
// mode: c++
// End:
