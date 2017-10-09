#ifndef art_Framework_Principal_RangeSetsSupported_h
#define art_Framework_Principal_RangeSetsSupported_h

#include "canvas/Persistency/Provenance/BranchType.h"

namespace art {
  namespace detail {
    template <BranchType>
    struct RangeSetsSupported : std::false_type {
    };

    template <>
    struct RangeSetsSupported<InRun> : std::true_type {
    };
    template <>
    struct RangeSetsSupported<InSubRun> : std::true_type {
    };
  }
}

#endif /* art_Framework_Principal_RangeSetsSupported_h */

// Local variables:
// mode: c++
// End:
