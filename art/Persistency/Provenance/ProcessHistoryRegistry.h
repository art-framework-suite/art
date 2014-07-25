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

  typedef  cet::registry_via_id<ProcessHistoryID,ProcessHistory>
           ProcessHistoryRegistry;

  static_assert(std::is_same<ProcessHistoryRegistry::collection_type, ProcessHistoryMap>::value,
                "ProcessHistoryRegistry::collection_type and ProcessHistoryMap should be the same type!");

}  // art

// ======================================================================

#endif /* art_Persistency_Provenance_ProcessHistoryRegistry_h */

// Local Variables:
// mode: c++
// End:
