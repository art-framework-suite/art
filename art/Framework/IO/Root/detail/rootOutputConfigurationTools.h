#ifndef art_Framework_IO_Root_detail_rootOutputConfigurationTools_h
#define art_Framework_IO_Root_detail_rootOutputConfigurationTools_h

#include "art/Framework/Core/OutputFileSwitchBoundary.h"

namespace art {
  namespace detail {

    void checkFileSwitchConfig(Boundary fileSwitchBoundary,
                               bool forceSwitch);

    Boundary checkMaxSizeConfig(bool switchBoundarySet,
                                Boundary switchBoundary,
                                bool forceSwitch);

    Boundary checkMaxEventsPerFileConfig(bool switchBoundarySet,
                                         Boundary switchBoundary,
                                         bool forceSwitch);

    bool shouldFastClone(bool fastCloningSet,
                         bool fastCloning,
                         bool wantAllEvents,
                         Boundary fileSwitchBoundary);

    bool shouldDropEvents(bool dropAllEventsSet,
                          bool dropAllEvents,
                          bool dropAllSubRuns);
  }
}

#endif

// Local variables:
// mode: c++
// End:
