#ifndef art_Utilities_ProductSemantics_h
#define art_Utilities_ProductSemantics_h

#include "canvas/Persistency/Provenance/RangeSet.h"

namespace art {

  template <Level>
  struct FullSemantic {};

  template <Level L>
  struct FragmentSemantic {};

  template <Level>
  struct RangedFragmentSemantic {
    explicit RangedFragmentSemantic(RangeSet const& r) : rs{r} {}
    RangeSet rs{RangeSet::invalid()};
  };

  // constexpr implies inline
  constexpr auto
  fullRun()
  {
    return FullSemantic<Level::Run>{};
  }
  constexpr auto
  fullSubRun()
  {
    return FullSemantic<Level::SubRun>{};
  }

  constexpr auto
  runFragment()
  {
    return FragmentSemantic<Level::Run>{};
  }
  constexpr auto
  subRunFragment()
  {
    return FragmentSemantic<Level::SubRun>{};
  }

  inline auto
  runFragment(RangeSet const& rs)
  {
    return RangedFragmentSemantic<Level::Run>{rs};
  }
  inline auto
  subRunFragment(RangeSet const& rs)
  {
    return RangedFragmentSemantic<Level::SubRun>{rs};
  }

} // namespace art

#endif /* art_Utilities_ProductSemantics_h */

// Local variables:
// mode: c++
// End:
