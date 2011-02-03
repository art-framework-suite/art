#ifndef art_Persistency_Provenance_EntryDescriptionRegistry_h
#define art_Persistency_Provenance_EntryDescriptionRegistry_h

// ======================================================================
//
// EntryDescriptionRegistry
//
// ======================================================================

#include "art/Persistency/Provenance/EventEntryDescription.h"
#include "art/Persistency/Provenance/EntryDescriptionID.h"
#include "cetlib/registry_via_id.h"


// Note that this registry is *not* directly persistable. The contents
// are persisted, but not the container.
namespace art {

  typedef  cet::registry_via_id<EntryDescriptionID, EventEntryDescription>
           EntryDescriptionRegistry;
  typedef  EntryDescriptionRegistry::collection_type
           EntryDescriptionMap;

}  // art

// ======================================================================

#endif /* art_Persistency_Provenance_EntryDescriptionRegistry_h */

// Local Variables:
// mode: c++
// End:
