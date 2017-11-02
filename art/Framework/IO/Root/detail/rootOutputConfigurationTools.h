#ifndef art_Framework_IO_Root_detail_rootOutputConfigurationTools_h
#define art_Framework_IO_Root_detail_rootOutputConfigurationTools_h

#include <string>

namespace art {

  class ClosingCriteria;

  namespace detail {

    bool shouldDropEvents(bool dropAllEventsSet,
                          bool dropAllEvents,
                          bool dropAllSubRuns);
  }
}

#endif /* art_Framework_IO_Root_detail_rootOutputConfigurationTools_h */

// Local variables:
// mode: c++
// End:
