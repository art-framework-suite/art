#include "art/Framework/IO/ProductMix/ProdToProdMapBuilder.h"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Utilities/Exception.h"

#include <iomanip>
#include <iostream>

void
art::ProdToProdMapBuilder::prepareTranslationTables(ProductIDTransMap& transMap)
{
  if (productIDTransMap_.empty()) {
    transMap.swap(productIDTransMap_);
  }
  else if (productIDTransMap_ != transMap) {
    throw Exception(errors::DataCorruption)
      << "Secondary input file has ProductIDs inconsistent with previous files.\n";
  }
}

void
art::ProdToProdMapBuilder::populateRemapper(PtrRemapper& mapper, Event const& e) const
{
  mapper.event_ = cet::make_exempt_ptr(&e);
  // Check translation map to see if output product IDs are supported
  // for given event.
  mapper.prodTransMap_ = productIDTransMap_;
}
