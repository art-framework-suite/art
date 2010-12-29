#ifndef DataFormats_Provenance_ParentageRegistry_h
#define DataFormats_Provenance_ParentageRegistry_h

// ======================================================================
//
// ParentageRegistry
//
// ======================================================================

#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ParentageID.h"
#include "cetlib/registry_via_id.h"

// ----------------------------------------------------------------------

// Note that this registry is *not* directly persistable. The contents
// are persisted, but not the container.
namespace art {

  typedef  cet::registry_via_id<ParentageID, Parentage>  ParentageRegistry;
  typedef  ParentageRegistry::collection_type            ParentageMap;

}  // art

// ======================================================================

#endif
