#ifndef art_Persistency_Provenance_BranchIDListRegistry_h
#define art_Persistency_Provenance_BranchIDListRegistry_h

#include "art/Utilities/ThreadSafeIndexedRegistry.h"
#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "canvas/Persistency/Provenance/BranchIDList.h"

namespace art {
  typedef art::detail::ThreadSafeIndexedRegistry<BranchIDList, BranchIDListHelper> BranchIDListRegistry;
  typedef BranchIDListRegistry::collection_type BranchIDLists;
}

#endif /* art_Persistency_Provenance_BranchIDListRegistry_h */

// Local Variables:
// mode: c++
// End:
