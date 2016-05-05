#ifndef art_Framework_Core_detail_ProductTokens_h
#define art_Framework_Core_detail_ProductTokens_h

#include "canvas/Persistency/Provenance/RangeSet.h"

namespace art {

  template <Level>
  struct FullToken {};

  template <Level L>
  struct FragmentToken {};

  template <Level>
  struct RangedFragmentToken {
    explicit RangedFragmentToken(RangeSet const& r): rs{r} {}
    RangeSet rs {RangeSet::invalid()};
  };

  // constexpr implies inline
  constexpr auto fullRun() { return FullToken<Level::Run>{}; }
  constexpr auto fullSubRun() { return FullToken<Level::SubRun>{}; }

  constexpr auto runFragment() { return FragmentToken<Level::Run>{}; }
  constexpr auto subRunFragment() { return FragmentToken<Level::SubRun>{}; }

  inline auto runFragment(RangeSet const& rs) { return RangedFragmentToken<Level::Run>{rs}; }
  inline auto subRunFragment(RangeSet const& rs) { return RangedFragmentToken<Level::SubRun>{rs}; }

}

#endif

// Local variables:
// mode: c++
// End:
