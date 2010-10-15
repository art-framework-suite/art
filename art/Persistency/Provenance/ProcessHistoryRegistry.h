#ifndef DataFormats_Provenance_ProcessHistoryRegistry_h
#define DataFormats_Provenance_ProcessHistoryRegistry_h

#include "art/Utilities/ThreadSafeRegistry.h"
#include "art/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProcessHistoryID.h"

namespace art
{
  typedef art::detail::ThreadSafeRegistry<art::ProcessHistoryID,art::ProcessHistory> ProcessHistoryRegistry;
  typedef ProcessHistoryRegistry::collection_type ProcessHistoryMap;
}

#endif
