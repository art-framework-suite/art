#ifndef DataFormats_Provenance_ProcessHistoryRegistry_h
#define DataFormats_Provenance_ProcessHistoryRegistry_h

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
  typedef  ProcessHistoryRegistry::collection_type
           ProcessHistoryMap;

}  // art

// ======================================================================

#endif
