#ifndef DataFormats_Provenance_ProcessConfigurationRegistry_h
#define DataFormats_Provenance_ProcessConfigurationRegistry_h

#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Persistency/Provenance/ProcessConfigurationID.h"
#include "cetlib/registry_via_id.h"

namespace art
{
  typedef cet::registry_via_id<art::ProcessConfigurationID,art::ProcessConfiguration> ProcessConfigurationRegistry;
  typedef ProcessConfigurationRegistry::collection_type ProcessConfigurationMap;
}

#endif  // DataFormats_Provenance_ProcessConfigurationRegistry_h
