#ifndef art_Persistency_Provenance_ProcessHistoryRegistry_h
#define art_Persistency_Provenance_ProcessHistoryRegistry_h

// ======================================================================
//
// ProcessHistoryRegistry
//
// ======================================================================

#include "art/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "cetlib/registry_via_id.h"

// ----------------------------------------------------------------------

namespace art {

  typedef  cet::registry_via_id<ProcessHistoryID, ProcessHistory>
  ProcessHistoryRegistry;
  typedef  ProcessHistoryRegistry::collection_type
  ProcessHistoryMap;

}  // art

// ======================================================================

#endif /* art_Persistency_Provenance_ProcessHistoryRegistry_h */

// Local Variables:
// mode: c++
// End:
