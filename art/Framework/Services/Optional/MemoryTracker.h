#ifndef art_Framework_Services_Optional_MemoryTracker_h
#define art_Framework_Services_Optional_MemoryTracker_h

#ifdef __linux__
#  include "art/Framework/Services/Optional/MemoryTrackerLinux.h"
#elif  __APPLE__
#  include "art/Framework/Services/Optional/MemoryTrackerDarwin.h"
#endif

#endif /* art_Framework_Services_Optional_MemoryTracker_h */
