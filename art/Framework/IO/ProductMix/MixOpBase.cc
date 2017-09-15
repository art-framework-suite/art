#include "art/Framework/IO/ProductMix/MixOpBase.h"
#include "canvas_root_io/Streamers/RefCoreStreamer.h"
#include "canvas_root_io/Streamers/ProductIDStreamer.h"

// Note: This is a separate function so that users can avoid having to
//       introduce a dependency change from canvas to canvas_root_io.

void
art::MixOpBase::configureStreamers(cet::exempt_ptr<BranchIDLists const> branchIDLists)
{
  // Make sure the schema evolution is ready for ProductID
  configureProductIDStreamer(branchIDLists);
  // Make sure we don't have a ProductGetter set.
  configureRefCoreStreamer();
}
