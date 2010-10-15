#ifndef DataFormats_Provenance_EntryDescriptionRegistry_h
#define DataFormats_Provenance_EntryDescriptionRegistry_h

#include "art/Utilities/ThreadSafeRegistry.h"
#include "art/Persistency/Provenance/EventEntryDescription.h"
#include "art/Persistency/Provenance/EntryDescriptionID.h"


// Note that this registry is *not* directly persistable. The contents
// are persisted, but not the container.
namespace art
{
  typedef art::detail::ThreadSafeRegistry<art::EntryDescriptionID, art::EventEntryDescription> EntryDescriptionRegistry;
  typedef EntryDescriptionRegistry::collection_type EntryDescriptionMap;
}

#endif
