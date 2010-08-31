#ifndef DataFormats_Provenance_ModuleDescriptionRegistry_h
#define DataFormats_Provenance_ModuleDescriptionRegistry_h

#include "art/Utilities/ThreadSafeRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ModuleDescriptionID.h"

namespace edm
{
  typedef edm::detail::ThreadSafeRegistry<edm::ModuleDescriptionID, edm::ModuleDescription> ModuleDescriptionRegistry;
  typedef ModuleDescriptionRegistry::collection_type ModuleDescriptionMap;
}

#endif
