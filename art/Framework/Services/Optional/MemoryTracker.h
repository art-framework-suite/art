#ifndef art_Framework_Services_Optional_MemoryTracker_h
#define art_Framework_Services_Optional_MemoryTracker_h

#include "art/Framework/Services/Registry/ServiceMacros.h"

#ifdef __linux__
#  include "art/Framework/Services/Optional/MemoryTrackerLinux.h"
#elif  __APPLE__
#  include "art/Framework/Services/Optional/MemoryTrackerDarwin.h"
#endif

DECLARE_ART_SERVICE(art::MemoryTracker, LEGACY)

#endif /* art_Framework_Services_Optional_MemoryTracker_h */

// Local Variables:
// mode: c++
// End:
