#ifndef art_Framework_IO_Root_detail_rootOutputConfigurationTools_h
#define art_Framework_IO_Root_detail_rootOutputConfigurationTools_h

#include <string>

namespace art {

  class ClosingCriteria;

  namespace detail {

    bool shouldFastClone(bool fastCloningSet,
                         bool fastCloning,
                         bool wantAllEvents,
                         ClosingCriteria const& fileProperties);

    bool shouldDropEvents(bool dropAllEventsSet,
                          bool dropAllEvents,
                          bool dropAllSubRuns);
  }
}

#endif /* art_Framework_IO_Root_detail_rootOutputConfigurationTools_h */

// Local variables:
// mode: c++
// End:
