#ifndef art_Persistency_Provenance_Selections_h
#define art_Persistency_Provenance_Selections_h

#include "canvas/Persistency/Provenance/BranchType.h"

#include <array>
#include <vector>

namespace art {
  class BranchDescription;
  using Selections = std::vector<BranchDescription const*>;
  using SelectionsArray = std::array<Selections, NumBranchTypes>;
}

#endif /* art_Persistency_Provenance_Selections_h */

// Local Variables:
// mode: c++
// End:
