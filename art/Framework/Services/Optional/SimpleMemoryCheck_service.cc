#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Optional/SimpleMemoryCheck.h"

// ======================================================================

// The DECLARE macro call should be moved to the header file, should you
// create one.
DECLARE_ART_SERVICE(art::SimpleMemoryCheck, LEGACY)
DEFINE_ART_SERVICE(art::SimpleMemoryCheck)

// ======================================================================
