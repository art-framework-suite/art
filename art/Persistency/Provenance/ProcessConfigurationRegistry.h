#ifndef art_Persistency_Provenance_ProcessConfigurationRegistry_h
#define art_Persistency_Provenance_ProcessConfigurationRegistry_h

// ======================================================================
//
// ProcessConfigurationRegistry
//
// ======================================================================

#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Persistency/Provenance/ProcessConfigurationID.h"
#include "cetlib/registry_via_id.h"

namespace art {

  typedef  cet::registry_via_id<ProcessConfigurationID,ProcessConfiguration>
           ProcessConfigurationRegistry;
  typedef  ProcessConfigurationRegistry::collection_type
           ProcessConfigurationMap;

}  // art

// ======================================================================

#endif /* art_Persistency_Provenance_ProcessConfigurationRegistry_h */

// Local Variables:
// mode: c++
// End:
