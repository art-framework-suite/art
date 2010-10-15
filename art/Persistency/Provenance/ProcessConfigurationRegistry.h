#ifndef DataFormats_Provenance_ProcessConfigurationRegistry_h
#define DataFormats_Provenance_ProcessConfigurationRegistry_h

#include "art/Utilities/ThreadSafeRegistry.h"
#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Persistency/Provenance/ProcessConfigurationID.h"

namespace art
{
  typedef art::detail::ThreadSafeRegistry<art::ProcessConfigurationID,art::ProcessConfiguration> ProcessConfigurationRegistry;
  typedef ProcessConfigurationRegistry::collection_type ProcessConfigurationMap;
}

#endif
