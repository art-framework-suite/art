#include "art/Persistency/Provenance/ProductRegistry.h"

void
art::ProductRegistry::initAllBranches() {
  for (ProductList::iterator
         i = productList_.begin(),
         e = productList_.end();
       i != e;
       ++i) {
    i->second.init();
  }
}
