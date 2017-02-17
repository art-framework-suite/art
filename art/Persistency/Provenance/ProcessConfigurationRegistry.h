#ifndef art_Persistency_Provenance_ProcessConfigurationRegistry_h
#define art_Persistency_Provenance_ProcessConfigurationRegistry_h

// ======================================================================
//
// ProcessConfigurationRegistry
//
// ======================================================================

#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Persistency/Provenance/ProcessConfigurationID.h"
#include "canvas/Persistency/Provenance/thread_safe_registry_via_id.h"

namespace art {

  using ProcessConfigurationRegistry = thread_safe_registry_via_id<ProcessConfigurationID,ProcessConfiguration>;
  using ProcessConfigurationMap = ProcessConfigurationRegistry::collection_type;

}  // art

// ======================================================================

#endif /* art_Persistency_Provenance_ProcessConfigurationRegistry_h */

// Local Variables:
// mode: c++
// End:
