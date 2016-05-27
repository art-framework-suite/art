#ifndef art_Framework_Services_Optional_MemoryTrackerDarwin_h
#define art_Framework_Services_Optional_MemoryTrackerDarwin_h

// ======================================================================
//
// MemoryTrackerDarwin
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceTable.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace fhicl { class ParameterSet; }

namespace art {

  class ActivityRegistry;

  class MemoryTracker {
  public:

    MemoryTracker(fhicl::ParameterSet const&, ActivityRegistry&) {
      mf::LogAbsolute("MemoryTracker") << "\n"
                                       << "The MemoryTracker service is not supported for this operating system.\n"
                                       << "If desired, please log an issue with:\n\n"
                                       << "https://cdcvs.fnal.gov/redmine/projects/cet-is/issues/new\n\n";
    }

  };

}  // art

#endif /* art_Framework_Services_Optional_MemoryTrackerDarwin_h */

// Local variables:
// mode: c++
// End:
