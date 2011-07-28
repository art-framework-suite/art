#ifndef art_Persistency_Provenance_ProductRegistry_h
#define art_Persistency_Provenance_ProductRegistry_h
#include "art/Persistency/Provenance/ProductList.h"

// Class solely to permit schema evolution from the old ProductRegistry
// directly to a ProductList.

namespace art {
  struct ProductRegistry;
}

struct art::ProductRegistry {
  ProductRegistry() : productList_() {} // For ROOT.
  ProductRegistry(ProductList const &pl) : productList_(pl) {}

  void initAllBranches();

  ProductList productList_;
};
#endif /* art_Persistency_Provenance_ProductRegistry_h */

// Local Variables:
// mode: c++
// End:
