#ifndef art_Persistency_Provenance_ProductList_h
#define art_Persistency_Provenance_ProductList_h
////////////////////////////////////////////////////////////////////////
// ProductList
//
// This is a very-badly-named typedef; please make it go away soon.
////////////////////////////////////////////////////////////////////////

#include <map>
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchKey.h"

namespace art {
  typedef std::map<BranchKey, BranchDescription> ProductList;
}

#endif /* art_Persistency_Provenance_ProductList_h */

// Local Variables:
// mode: c++
// End:
