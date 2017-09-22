#ifndef art_Persistency_Provenance_Selections_h
#define art_Persistency_Provenance_Selections_h

#include "canvas/Persistency/Provenance/BranchType.h"

#include <array>
#include <set>

namespace art {
  class BranchDescription;
  using Selections = std::set<BranchDescription>;
  using SelectionsArray = std::array<Selections, NumBranchTypes>;
}

#endif /* art_Persistency_Provenance_Selections_h */

// Local Variables:
// mode: c++
// End:
