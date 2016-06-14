#ifndef art_Persistency_Provenance_EventSelectionID_h
#define art_Persistency_Provenance_EventSelectionID_h

// ======================================================================
//
// EventSelectionID - An identifier to uniquely identify the
//                    configuration of the event selector subsystem of an
//                    OutputModule.
//
// ======================================================================

#include "fhiclcpp/ParameterSetID.h"
#include <vector>

namespace art {

  typedef  fhicl::ParameterSetID          EventSelectionID;
  typedef  std::vector<EventSelectionID>  EventSelectionIDVector;

}  // art

// ======================================================================

#endif /* art_Persistency_Provenance_EventSelectionID_h */

// Local Variables:
// mode: c++
// End:
