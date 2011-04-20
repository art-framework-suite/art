#include "art/Framework/IO/ProductMerge/MergeHelper.h"

art::MergeHelper::MergeHelper(ProducerBase &producesProvider)
  :
  producesProvider_(producesProvider),
  mergeOps_()
{}

art::ProducerBase &
art::MergeHelper::producesProvider() const {
  return producesProvider_;
}

