#ifndef art_Framework_Art_detail_LibraryInfoCollection_h
#define art_Framework_Art_detail_LibraryInfoCollection_h

#include "art/Framework/Art/detail/LibraryInfo.h"

#include <string>
#include <set>

namespace art {
  namespace detail {

    using LibraryInfoCollection = std::multiset<LibraryInfo>;

  }
}

#endif /* art_Framework_Art_detail_LibraryInfoCollection_h */

// Local variables:
// mode: c++
// End:
