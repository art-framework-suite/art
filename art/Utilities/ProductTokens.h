#ifndef art_Framework_Core_detail_ProductTokens_h
#define art_Framework_Core_detail_ProductTokens_h

#include "canvas/Persistency/Provenance/RangeSet.h"

namespace art {
  namespace detail {

    template <Level>
    struct FullToken {};

    template <Level>
    struct RangedFragmentToken {
      explicit RangedFragmentToken(RangeSet const& r): rs{r} {}
      RangeSet rs {RangeSet::invalid()};
    };

    template <Level L>
    struct FragmentToken {
      auto operator()(RangeSet const& r) const { return RangedFragmentToken<L>{r}; }
    };

  }
}

#endif

// Local variables:
// mode: c++
// End:
