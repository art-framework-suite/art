#ifndef FWCore_Framework_BranchIDListRegistry_h
#define FWCore_Framework_BranchIDListRegistry_h

#include "art/Utilities/ThreadSafeIndexedRegistry.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/BranchIDList.h"

namespace edm {
  typedef edm::detail::ThreadSafeIndexedRegistry<BranchIDList, BranchIDListHelper> BranchIDListRegistry;
  typedef BranchIDListRegistry::collection_type BranchIDLists;
}

#endif
