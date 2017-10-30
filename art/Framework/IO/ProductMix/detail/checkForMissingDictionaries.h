#ifndef art_Framework_IO_ProductMix_detail_checkForMissingDictionaries_h
#define art_Framework_IO_ProductMix_detail_checkForMissingDictionaries_h

#include "canvas/Utilities/TypeID.h"
#include <vector>

namespace art {
  namespace detail {
    void checkForMissingDictionaries(std::vector<TypeID> const& types) noexcept(
      false);
  }
}

#endif /* art_Framework_IO_ProductMix_detail_checkForMissingDictionaries_h */

// Local Variables:
// mode: c++
// End:
