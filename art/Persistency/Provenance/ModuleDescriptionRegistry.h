#ifndef DataFormats_Provenance_ModuleDescriptionRegistry_h
#define DataFormats_Provenance_ModuleDescriptionRegistry_h

#include "art/Utilities/ThreadSafeRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ModuleDescriptionID.h"

namespace art
{
  typedef art::detail::ThreadSafeRegistry<art::ModuleDescriptionID, art::ModuleDescription> ModuleDescriptionRegistry;
  typedef ModuleDescriptionRegistry::collection_type ModuleDescriptionMap;
}

#endif
