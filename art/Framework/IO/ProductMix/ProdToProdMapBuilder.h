#ifndef art_Framework_IO_ProductMix_ProdToProdMapBuilder_h
#define art_Framework_IO_ProductMix_ProdToProdMapBuilder_h

#include "art/Framework/Core/PtrRemapper.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "cetlib/exempt_ptr.h"

#include <functional>
#include <map>

class TTree;

namespace art {
  class ProdToProdMapBuilder;
}

class art::ProdToProdMapBuilder {
public:
  using ProductIDTransMap = std::map<ProductID, ProductID>;

  void prepareTranslationTables(ProductIDTransMap& transMap);
  PtrRemapper getRemapper(Event const& e) const;

private:
  ProductIDTransMap productIDTransMap_{};
};
#endif /* art_Framework_IO_ProductMix_ProdToProdMapBuilder_h */

// Local Variables:
// mode: c++
// End:
