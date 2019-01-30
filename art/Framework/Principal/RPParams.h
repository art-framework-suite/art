#ifndef art_Framework_Principal_RPParams_h
#define art_Framework_Principal_RPParams_h
// vim: set sw=2 expandtab :

////////////////////////////////////////////////////////////////////////
// RPParams
//
// Parameters pertinent to a particular ResultsProducer passed via the
// RPWorker.
////////////////////////////////////////////////////////////////////////

#include "fhiclcpp/ParameterSetID.h"

#include <string>

namespace art {

  struct RPParams {
    fhicl::ParameterSetID rpPSetID;
    std::string rpPluginType;
    std::string rpLabel;
  };

} // namespace art

#endif /* art_Framework_Principal_RPParams_h */

// Local Variables:
// mode: c++
// End:
