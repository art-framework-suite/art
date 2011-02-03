#ifndef art_Persistency_Provenance_Selections_h
#define art_Persistency_Provenance_Selections_h

#include "boost/array.hpp"
#include <vector>

#include "art/Persistency/Provenance/BranchType.h"

namespace art {
  class BranchDescription;
  typedef std::vector<BranchDescription const *> Selections;
  typedef boost::array<Selections, NumBranchTypes> SelectionsArray;
}

#endif /* art_Persistency_Provenance_Selections_h */

// Local Variables:
// mode: c++
// End:
