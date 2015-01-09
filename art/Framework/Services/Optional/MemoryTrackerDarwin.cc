// ======================================================================
//
// MemoryTrackerDarwin
//
// ======================================================================

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <type_traits>

//====================================================================================

namespace art {

#ifndef __APPLE__
  static_assert( 0, "MemoryTracker(Darwin) incompatible with this operating system." );
#endif

  class MemoryTrackerDarwin {
  public:

    MemoryTrackerDarwin(fhicl::ParameterSet const &, ActivityRegistry &) {
      mf::LogAbsolute("MemoryTracker") << "\n"
                                       << "Service currently not supported for this operating system.\n"
                                       << "If desired, please log an issue with:\n\n"
                                       << "https://cdcvs.fnal.gov/redmine/projects/cet-is/issues/new\n\n";
    }

  }; // MemoryTrackerDarwin

}  // art

