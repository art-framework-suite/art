#ifndef art_Persistency_Provenance_Selections_h
#define art_Persistency_Provenance_Selections_h

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"

#include <array>
#include <set>

namespace art {
  using Selections = ProductDescriptionsByID;
  using SelectionsArray = std::array<Selections, NumBranchTypes>;
} // namespace art

#endif /* art_Persistency_Provenance_Selections_h */

// Local Variables:
// mode: c++
// End:
