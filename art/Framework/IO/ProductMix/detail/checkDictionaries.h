#ifndef art_Framework_IO_ProductMix_detail_checkDictionaries_h
#define art_Framework_IO_ProductMix_detail_checkDictionaries_h

#include "art/Utilities/TypeID.h"
#include <vector>

namespace art {
  namespace detail {
    void checkForMissingDictionaries(std::vector<TypeID> const& types);
  }
}

#endif
