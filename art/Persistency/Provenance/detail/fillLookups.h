#ifndef art_Persistency_Provenance_detail_fillLookups_h
#define art_Persistency_Provenance_detail_fillLookups_h

#include "art/Persistency/Provenance/detail/type_aliases.h"
#include "canvas/Persistency/Provenance/ProductList.h"

#include <utility>

namespace art {
  namespace detail {
    std::pair<ProductLookup_t, ViewLookup_t> fillLookups(ProductList const& products);
  }
}

#endif
