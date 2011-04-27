#include "art/Framework/IO/ProductMerge/MergeHelper.h"
#include "art/Framework/IO/ProductMerge/SecondaryEventSequence.h"
#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/setMetaDataBranchAddress.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/ConstProductRegistry.h"

art::MergeHelper::MergeHelper(fhicl::ParameterSet const &pset,
                              ProducerBase &producesProvider)
  :
  producesProvider_(producesProvider),
  filenames_(pset.get<std::vector<std::string> >("filenames")),
  mergeOps_(),
  ptrRemapper_(),
  currentFilename_(filenames_.begin()),
  readMode_(pset.get<std::string>("readMode", "sequential")),
  coverageFraction_(pset.get<double>("coverageFraction", 100.0)),
  nEventsRead_(0),
  ffVersion_(),
  pReg_(),
  dist_(ServiceHandle<RandomNumberGenerator>()->getEngine()),
  currentFile_(),
  currentMetaDataTree_(),
  currentEventTree_(),
  fileIndex_()
{
}

void
art::MergeHelper::openAndReadMetaData(std::string const &filename) {
  // Open file.
  try {
    currentFile_.reset(TFile::Open(filename.c_str()));
  }
  catch (std::exception const &e) {
    throw Exception(errors::FileOpenError, e.what())
      << "Unable to open specified secondary event stream file "
      << filename
      << ".\n";
    
  }
  if (!currentFile_ || currentFile_->IsZombie()) {
    throw Exception(errors::FileOpenError)
      << "Unable to open specified secondary event stream file "
      << filename
      << ".\n";
  }

  // Obtain meta data tree.
  currentMetaDataTree_.reset(dynamic_cast<TTree *>
                             (currentFile_->
                              Get(rootNames::metaDataTreeName().c_str())));
  if (currentMetaDataTree_.get() == 0) {
    throw Exception(errors::FileReadError)
      << "Unable to read meta data tree from secondary event stream file "
      << filename
      << ".\n";
  }

  // Obtain event tree.
  currentEventTree_.reset(dynamic_cast<TTree *>
                          (currentFile_->
                           Get(rootNames::eventTreeName().c_str())));
  if (currentEventTree_.get() == 0) {
    throw Exception(errors::FileReadError)
      << "Unable to read event tree from secondary event stream file "
      << filename
      << ".\n";
  }
  
  // Read meta data
  FileFormatVersion *ffVersion_p = &ffVersion_;
  setMetaDataBranchAddress(currentMetaDataTree_, ffVersion_p);

  FileIndex *fileIndex_p = &fileIndex_;
  setMetaDataBranchAddress(currentMetaDataTree_, fileIndex_p);

  ProductRegistry *pReg_p = &pReg_;
  setMetaDataBranchAddress(currentMetaDataTree_, pReg_p);

  Int_t n = currentMetaDataTree_->GetEntry(0);
  switch (n) {
  case -1:
    throw Exception(errors::FileReadError)
      << "Apparent I/O error reading meta data information from secondary event stream file "
      << filename
      << ".\n";
  case 0:
    throw Exception(errors::FileReadError)
      << "Meta data tree apparently empty reading secondary event stream file "
      << filename
      << ".\n";
  }

  // Check file format era.
  std::string const expected_era = getFileFormatEra();
  if (ffVersion_.era_ != expected_era) {
    throw Exception(errors::FileReadError)
      << "Can only read files written during the \""
      << expected_era << "\" era: "
      << "Era of "
      << "\"" << filename
      << "\" was "
      << (ffVersion_.era_.empty()?
          "not set":
          ("set to \"" + ffVersion_.era_ + "\" "))
      << ".\n";
  }

}

void
art::MergeHelper::postRegistrationInit() {
  // Open and read the first file to read branch information.
  openAndReadMetaData(*currentFilename_);
  for(MergeOpList::const_iterator
        i = mergeOps_.begin(),
        e = mergeOps_.end();
      i != e;
      ++i) {
    
  }
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
