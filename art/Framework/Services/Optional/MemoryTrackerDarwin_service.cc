// ======================================================================
//
// MemoryTrackerDarwin
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceDeclarationMacros.h"
#include "art/Framework/Services/Registry/ServiceDefinitionMacros.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace fhicl {
  class ParameterSet;
}

namespace art {

  class MemoryTracker {
  public:
    static constexpr bool service_handle_allowed{false};

    MemoryTracker(fhicl::ParameterSet const&)
    {
      mf::LogAbsolute("MemoryTracker")
        << '\n'
        << "The MemoryTracker service is not supported for this operating "
           "system.\n"
        << "If desired, please log an issue with:\n\n"
        << "https://cdcvs.fnal.gov/redmine/projects/cet-is/issues/new\n\n";
    }
  };
} // namespace art

DECLARE_ART_SERVICE(art::MemoryTracker, SHARED)
DEFINE_ART_SERVICE(art::MemoryTracker)
