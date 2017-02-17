#ifndef art_Persistency_Provenance_ProcessHistoryRegistry_h
#define art_Persistency_Provenance_ProcessHistoryRegistry_h

// ======================================================================
//
// ProcessHistoryRegistry
//
// ======================================================================

#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProcessHistoryID.h"
#include "canvas/Persistency/Provenance/thread_safe_registry_via_id.h"

namespace art {
  using ProcessHistoryRegistry = thread_safe_registry_via_id<ProcessHistoryID, ProcessHistory>;
}

// ======================================================================

#endif /* art_Persistency_Provenance_ProcessHistoryRegistry_h */

// Local Variables:
// mode: c++
// End:
