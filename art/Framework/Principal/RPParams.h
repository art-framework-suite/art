#ifndef art_Framework_Principal_RPParams_h
#define art_Framework_Principal_RPParams_h

#include "fhiclcpp/ParameterSetID.h"

#include <string>

namespace art {
  class RPParams;
}

struct art::RPParams {
  fhicl::ParameterSetID psetID;
  std::string rpPluginType;
  std::string rpLabel;
};

#endif /* art_Framework_Principal_RPParams_h */

// Local Variables:
// mode: c++
// End:
