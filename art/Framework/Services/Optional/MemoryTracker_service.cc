#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Optional/MemoryTracker.h"

// ======================================================================

// The DECLARE macro call should be moved to the header file, should you
// create one.
DECLARE_ART_SERVICE(art::MemoryTracker, LEGACY)
DEFINE_ART_SERVICE(art::MemoryTracker)

// ======================================================================
