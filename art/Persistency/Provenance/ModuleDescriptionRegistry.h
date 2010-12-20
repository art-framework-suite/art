#ifndef DataFormats_Provenance_ModuleDescriptionRegistry_h
#define DataFormats_Provenance_ModuleDescriptionRegistry_h

#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/ModuleDescriptionID.h"
#include "cetlib/registry_via_id.h"

namespace art
{
  typedef cet::registry_via_id<art::ModuleDescriptionID, art::ModuleDescription> ModuleDescriptionRegistry;
  typedef ModuleDescriptionRegistry::collection_type ModuleDescriptionMap;
}

#endif  // DataFormats_Provenance_ModuleDescriptionRegistry_h
