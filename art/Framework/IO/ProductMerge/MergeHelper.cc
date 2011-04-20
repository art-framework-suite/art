#include "art/Framework/IO/ProductMerge/MergeHelper.h"

art::MergeHelper::MergeHelper(ProducerBase &producesProvider)
  :
  producesProvider_(producesProvider),
  mergeOps_(),
  ptrRemapper_()
{}

void
art::MergeHelper::mergeAndPut(size_t nSecondaries, Event &e) {
}
