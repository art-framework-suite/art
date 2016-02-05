#ifndef art_Persistency_Provenance_ParentageRegistry_h
#define art_Persistency_Provenance_ParentageRegistry_h

// ======================================================================
//
// ParentageRegistry
//
// ======================================================================

#include "canvas/Persistency/Provenance/Parentage.h"
#include "canvas/Persistency/Provenance/ParentageID.h"
#include "cetlib/registry_via_id.h"

// ----------------------------------------------------------------------

// Note that this registry is *not* directly persistable. The contents
// are persisted, but not the container.
namespace art {

  typedef  cet::registry_via_id<ParentageID, Parentage>  ParentageRegistry;
  typedef  ParentageRegistry::collection_type            ParentageMap;

}  // art

// ======================================================================

#endif /* art_Persistency_Provenance_ParentageRegistry_h */

// Local Variables:
// mode: c++
// End:
