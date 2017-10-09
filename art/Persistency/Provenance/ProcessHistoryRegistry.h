#ifndef art_Persistency_Provenance_ProcessHistoryRegistry_h
#define art_Persistency_Provenance_ProcessHistoryRegistry_h

// ======================================================================
//
// ProcessHistoryRegistry
//
// This registry must be able to support concurrent writing and
// reading.  For the foreseeable future, only concurrent insertion
// will be necessary, however concurrent reading is also tested.  In
// the event that an insertion can happen concurrently with a read,
// care must be taken to determine if locking is required to ensure
// serialized access to the registry.
//
// ======================================================================

#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProcessHistoryID.h"
#include "canvas/Persistency/Provenance/thread_safe_registry_via_id.h"

namespace art {
  using ProcessHistoryRegistry =
    thread_safe_registry_via_id<ProcessHistoryID, ProcessHistory>;
}

  // ======================================================================

#endif /* art_Persistency_Provenance_ProcessHistoryRegistry_h */

// Local Variables:
// mode: c++
// End:
