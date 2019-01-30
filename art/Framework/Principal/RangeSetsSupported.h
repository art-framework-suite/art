#ifndef art_Framework_Principal_RangeSetsSupported_h
#define art_Framework_Principal_RangeSetsSupported_h

#include "canvas/Persistency/Provenance/BranchType.h"

#include <type_traits>

namespace art::detail {
  constexpr bool
  range_sets_supported(BranchType const bt)
  {
    return bt == InRun || bt == InSubRun;
  }
}

#endif /* art_Framework_Principal_RangeSetsSupported_h */

// Local variables:
// mode: c++
// End:
