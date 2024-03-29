#include "art/Framework/IO/ProductMix/ProdToProdMapBuilder.h"

#include "canvas/Utilities/Exception.h"
#include "cetlib/exempt_ptr.h"

void
art::ProdToProdMapBuilder::prepareTranslationTables(ProductIDTransMap& transMap)
{
  if (productIDTransMap_.empty()) {
    transMap.swap(productIDTransMap_);
  } else if (productIDTransMap_ != transMap) {
    throw Exception(errors::DataCorruption) << "Secondary input file has "
                                               "ProductIDs inconsistent with "
                                               "previous files.\n";
  }
}

art::PtrRemapper
art::ProdToProdMapBuilder::getRemapper(Event const& e) const
{
  PtrRemapper result;
  result.event_ = cet::make_exempt_ptr(&e);
  // Check translation map to see if output product IDs are supported
  // for given event.
  result.prodTransMap_ = productIDTransMap_;
  return result;
}
