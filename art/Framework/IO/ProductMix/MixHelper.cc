#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/IO/ProductMix/MixHelper.h"
#include "art/Framework/IO/ProductMix/SecondaryEventSequence.h"
#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/setMetaDataBranchAddress.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Persistency/Provenance/History.h"

#include <functional>

#include "Rtypes.h"

art::MixHelper::MixHelper(fhicl::ParameterSet const &pset,
                          ProducerBase &producesProvider)
  :
  producesProvider_(producesProvider),
  filenames_(pset.get<std::vector<std::string> >("filenames")),
  mixOps_(),
  branchIDTransMap_(),
  secondaryProductMap_(),
  ptrRemapper_(),
  currentFilename_(filenames_.begin()),
  readMode_(pset.get<std::string>("readMode", "sequential")),
  coverageFraction_(pset.get<double>("coverageFraction", 100.0)),
  nEventsRead_(0),
  ffVersion_(),
  branchIDToIndexMap_(),
  dist_(ServiceHandle<RandomNumberGenerator>()->getEngine()),
  currentFile_(),
  currentMetaDataTree_(),
  currentEventTree_(),
  fileIndex_(),
  dataBranches_()
{
}

art::MixHelper::SecondaryBranchIDToProductIDConverter::
SecondaryBranchIDToProductIDConverter(BranchIDToIndexMap const &bidi, History const &h)
  :
  bidi_(bidi),
  branchToProductIDHelper_()
{
  for(BranchListIndexes::const_iterator
        bli_beg = h.branchListIndexes().begin(),
        bli_it = bli_beg,
        bli_end = h.branchListIndexes().end();
      bli_it != bli_end;
      ++bli_it) {
    ProcessIndex pix = bli_it - bli_beg;
    branchToProductIDHelper_.insert(std::make_pair(*bli_it, pix));
  }
}

art::MixHelper::SecondaryBranchIDToProductIDConverter::result_type
art::MixHelper::SecondaryBranchIDToProductIDConverter::
operator()(argument_type bID) const {
  BranchIDToIndexMap::const_iterator bidi_it = bidi_.find(bID.first);
  if (bidi_it == bidi_.end()) {
    throw Exception(errors::NotFound, "Bad BranchID")
      << "Cannot determine secondary ProductID from BranchID.\n";
  }
  BLItoPIMap::const_iterator i =
    branchToProductIDHelper_.find(bidi_it->second.first);
  if (i == branchToProductIDHelper_.end()) {
    throw Exception(errors::NotFound, "Bad BranchID")
      << "Cannot determine secondary ProductID from BranchID.\n";
  }
  return std::make_pair(bID.first,
                        ProductID(i->second+1, bidi_it->second.second));
}

art::MixHelper::ProdTransMapBuilder::
ProdTransMapBuilder(BtoPTransMap const & spMap,
                    EventPrincipal const &ep)
  :
  spMap_(spMap),
  ep_(ep)
{}


art::MixHelper::ProdTransMapBuilder::result_type
art::MixHelper::ProdTransMapBuilder::
operator()(argument_type bIDs) const {
  BtoPTransMap::const_iterator secondary_i = spMap_.find(bIDs.first);
  if (secondary_i == spMap_.end()) {
    throw Exception(errors::NotFound)
      << "Unable to find product translation from secondary file.\n";
  }
  return std::make_pair(secondary_i->second, ep_.branchIDToProductID(bIDs.second));
}

void
art::MixHelper::openAndReadMetaData(std::string const &filename) {
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

  BranchIDLists branchIDLists;
  BranchIDLists *branchIDLists_p = &branchIDLists;
  setMetaDataBranchAddress(currentMetaDataTree_, branchIDLists_p);
  
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

  dataBranches_.reset(currentEventTree_.get());

  buildBranchIDTransMap();

  buildBranchIDToIndexMap(branchIDLists);

  buildSecondaryProductMap();
}

void
art::MixHelper::postRegistrationInit() {
  // Open and read the first file to read branch information.
  openAndReadMetaData(*currentFilename_);
}

void
art::MixHelper::mixAndPut(size_t nSecondaries, Event &e) {
  // Populate the remapper in case we need to remap any Ptrs.
  populateRemapper(e);

  // Decide which events we're reading and prime the event tree cache.
  SecondaryEventSequence seq;

  // Do the branch-wise read, mix and put.
  cet::for_all(mixOps_,
               std::bind(&MixHelper::mixAndPutOne,
                         this,
                         _1,
                         std::ref(seq),
                         nSecondaries,
                         std::ref(e)));
}

void
art::MixHelper::mixAndPutOne(boost::shared_ptr<MixOpBase> op,
                             SecondaryEventSequence const &seq,
                             size_t nSecondaries, Event &e) {
  op->readFromFile(currentEventTree_, seq, nSecondaries);
  op->mixAndPut(e, ptrRemapper_);
}

void
art::MixHelper::buildBranchIDTransMap() {
  BranchIDTransMap transMap;
  for(MixOpList::const_iterator
        i = mixOps_.begin(),
        e = mixOps_.end();
      i != e;
      ++i) {
    transMap[(*i)->incomingBranchID(dataBranches_)] = (*i)->outgoingBranchID();
  }
  if (branchIDTransMap_.empty()) {
    transMap.swap(branchIDTransMap_);
  } else if (branchIDTransMap_ != transMap) {
    throw Exception(errors::DataCorruption)
      << "Secondary input file "
      << *currentFilename_
      << " has BranchIDs inconsistent with previous files.\n";
  }
}

void
art::MixHelper::buildBranchIDToIndexMap(BranchIDLists const &bidl) {
  BranchIDToIndexMap tmpMap;
  for (BranchIDLists::const_iterator
         bidl_beg = bidl.begin(),
         bidl_it = bidl_beg,
         bidl_end = bidl.end();
       bidl_it != bidl.end();
       ++bidl_it) {
    BranchListIndex blix = bidl_it - bidl_beg;
    for (BranchIDList::const_iterator
           bids_beg = bidl_it->begin(),
           bids_it = bidl_it->begin(),
           bids_end = bidl_it->end();
         bids_it != bids_end;
         ++bids_it) {
      ProductIndex pix = bids_it - bids_beg;
      tmpMap.insert(std::make_pair(*bids_it, std::make_pair(blix, pix)));
    }
  }
  tmpMap.swap(branchIDToIndexMap_);
}

void
art::MixHelper::buildSecondaryProductMap() {
  TTree* ehTree = 
    dynamic_cast<TTree*>(currentFile_->Get(rootNames::eventHistoryTreeName().c_str()));
  if (ehTree == 0) {
    throw Exception(errors::FileReadError)
      << "Unable to read event history tree from secondary event stream file "
      << *currentFilename_
      << ".\n";
  }
  History h;
  History *h_p = &h;
  setMetaDataBranchAddress(ehTree, h_p);
  Long64_t nEvt = -1;
  BtoPTransMap tmpProdMap;
  while (Int_t n = ehTree->GetEntry(++nEvt)) {
    if (n > 0) {
      std::transform(branchIDTransMap_.begin(),
                     branchIDTransMap_.end(),
                     std::inserter(tmpProdMap, tmpProdMap.begin()),
                     SecondaryBranchIDToProductIDConverter(branchIDToIndexMap_, h));
    } else {
    }
    if (secondaryProductMap_.empty()) {
      tmpProdMap.swap(secondaryProductMap_);
    } else if (tmpProdMap != secondaryProductMap_) {
      throw Exception(errors::UnimplementedFeature)
        << "Unable to mix from a secondary input file which has skimmed events.\n"
        << "Contact artists@fnal.gov if you really need this feature.\n";
    }
  }
}

void
art::MixHelper::populateRemapper(Event &e) {
  ptrRemapper_.setProductGetter(cet::exempt_ptr<EDProductGetter const>(e.productGetter()));

  PtrRemapper::ProdTransMap_t prodTransMap;
  
  std::transform(branchIDTransMap_.begin(),
                 branchIDTransMap_.end(),
                 std::inserter(prodTransMap, prodTransMap.begin()),
                 ProdTransMapBuilder
                 (secondaryProductMap_,
                  *dynamic_cast<EventPrincipal const *>
                  (e.productGetter())
                  )
                 );

  ptrRemapper_.setProdTransMap(cet::exempt_ptr<PtrRemapper::ProdTransMap_t const>(&prodTransMap));
}
