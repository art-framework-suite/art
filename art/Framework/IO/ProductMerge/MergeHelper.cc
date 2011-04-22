#include "art/Framework/IO/ProductMerge/MergeHelper.h"
#include "art/Framework/IO/ProductMerge/SecondaryEventSequence.h"

art::MergeHelper::MergeHelper(fhicl::ParameterSet const &pset,
                              ProducerBase &producesProvider)
  :
  producesProvider_(producesProvider),
  fileNames_(pset.get<std::vector<std::string> >("fileNames")),
  mergeOps_(),
  ptrRemapper_(),
  currentFileName_(fileNames_.begin()),
  readMode_(pset.get<std::string>("readMode", "sequential")),
  coverageFraction_(pset.get<double>("coverageFraction", 100.0)),
  nEventsRead_(0),
  pReg_(),
  currentFile_(),
  currentMetaDataTree_(),
  currentEventTree_(),
  fileIndex_()
{
}

void
art::MergeHelper::openAndReadMetaData(std::string const &fileName) {
  // Open file and read metadata tree.
}

void
art::MergeHelper::postRegistrationInit() {
  // Open and read the first file to read branch information.
  openAndReadMetaData(*currentFileName_);
  // Fill the PtrRemapper translation table here
}

void
art::MergeHelper::mergeAndPut(size_t nSecondaries, Event &e) {
  // Set the product getter in case we need to remap any Ptrs.
  ptrRemapper_.setProductGetter(e.productGetter());

  // Decide which events we're reading and prime the event tree cache.
  SecondaryEventSequence seq;

  // Do the branch-wise read, merge and put.
  cet::for_all(mergeOps_,
               std::bind(&MergeHelper::mergeAndPutOne,
                         this,
                         _1,
                         std::ref(seq),
                         nSecondaries,
                         std::ref(e)));
}

void
art::MergeHelper::mergeAndPutOne(boost::shared_ptr<MergeOpBase> op,
                                 SecondaryEventSequence const &seq,
                                 size_t nSecondaries, Event &e) {
  op->readFromFile(currentEventTree_, seq, nSecondaries);
  op->mergeAndPut(e, ptrRemapper_);
}
