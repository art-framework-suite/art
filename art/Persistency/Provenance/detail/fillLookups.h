#ifndef art_Persistency_Provenance_detail_fillLookups_h
#define art_Persistency_Provenance_detail_fillLookups_h

#include "art/Persistency/Provenance/detail/type_aliases.h"
#include "canvas/Persistency/Provenance/ProductList.h"

namespace art {
  namespace detail {
    void fillLookups(ProductList const& products,
                     BranchTypeLookup& pl,
                     BranchTypeLookup& el);
  }
}

#endif
