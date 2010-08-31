#ifndef DataFormats_Provenance_ProcessConfigurationRegistry_h
#define DataFormats_Provenance_ProcessConfigurationRegistry_h

#include "art/Utilities/ThreadSafeRegistry.h"
#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Persistency/Provenance/ProcessConfigurationID.h"

namespace edm
{
  typedef edm::detail::ThreadSafeRegistry<edm::ProcessConfigurationID,edm::ProcessConfiguration> ProcessConfigurationRegistry;
  typedef ProcessConfigurationRegistry::collection_type ProcessConfigurationMap;
}

#endif
