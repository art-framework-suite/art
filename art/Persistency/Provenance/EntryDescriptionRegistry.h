#ifndef DataFormats_Provenance_EntryDescriptionRegistry_h
#define DataFormats_Provenance_EntryDescriptionRegistry_h

#include "art/Persistency/Provenance/EventEntryDescription.h"
#include "art/Persistency/Provenance/EntryDescriptionID.h"
#include "cetlib/registry_via_id.h"


// Note that this registry is *not* directly persistable. The contents
// are persisted, but not the container.
namespace art
{
  typedef cet::registry_via_id<art::EntryDescriptionID, art::EventEntryDescription> EntryDescriptionRegistry;
  typedef EntryDescriptionRegistry::collection_type EntryDescriptionMap;
}

#endif  // DataFormats_Provenance_EntryDescriptionRegistry_h
