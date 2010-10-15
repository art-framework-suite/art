#ifndef DataFormats_Provenance_ParentageRegistry_h
#define DataFormats_Provenance_ParentageRegistry_h

#include "art/Utilities/ThreadSafeRegistry.h"
#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ParentageID.h"


// Note that this registry is *not* directly persistable. The contents
// are persisted, but not the container.
namespace art
{
  typedef art::detail::ThreadSafeRegistry<art::ParentageID, art::Parentage> ParentageRegistry;
  typedef ParentageRegistry::collection_type ParentageMap;
}

#endif
