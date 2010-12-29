#ifndef DataFormats_Provenance_ProcessConfigurationRegistry_h
#define DataFormats_Provenance_ProcessConfigurationRegistry_h

// ======================================================================
//
// ProcessConfigurationRegistry
//
// ======================================================================

#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Persistency/Provenance/ProcessConfigurationID.h"
#include "cetlib/registry_via_id.h"

namespace art {

  typedef  cet::registry_via_id<ProcessConfigurationID,ProcessConfiguration>
           ProcessConfigurationRegistry;
  typedef  ProcessConfigurationRegistry::collection_type
           ProcessConfigurationMap;

}  // art

// ======================================================================

#endif
