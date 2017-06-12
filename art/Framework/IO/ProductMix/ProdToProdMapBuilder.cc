#include "art/Framework/IO/ProductMix/ProdToProdMapBuilder.h"

#include "Rtypes.h"
#include "TTree.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Utilities/Exception.h"

#include <iomanip>
#include <iostream>

void
art::ProdToProdMapBuilder::prepareTranslationTables(BranchIDTransMap& transMap)
{
  if (branchIDTransMap_.empty()) {
    transMap.swap(branchIDTransMap_);
  }
  else if (branchIDTransMap_ != transMap) {
    throw Exception(errors::DataCorruption)
      << "Secondary input file "
      " has BranchIDs inconsistent with previous files.\n";
  }
}

void
art::ProdToProdMapBuilder::populateRemapper(PtrRemapper& mapper, Event& e) const
{
  mapper.event_.reset(&e);
  mapper.prodTransMap_.clear();
  std::transform(branchIDTransMap_.begin(),
                 branchIDTransMap_.end(),
                 std::inserter(mapper.prodTransMap_, mapper.prodTransMap_.begin()),
                 [](auto const& pr){ return std::make_pair(ProductID{pr.first.id()}, ProductID{pr.second.id()}); });
#if ART_DEBUG_PTRREMAPPER
  for (auto const& pr : mapper.prodTransMap_) {
    std::cerr << "ProdTransMap_t: "
              << "("
              << pr.first.value()
              << ") -> ("
              << pr.second.value()
              << ").\n";
  }
#endif
}
