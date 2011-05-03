#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/IO/ProductMix/MixHelper.h"
#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/setMetaDataBranchAddress.h"
#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Persistency/Provenance/History.h"

#include "boost/regex.hpp"

#include <functional>

#include "Rtypes.h"

namespace {
  class EventIDIndexBuilder :
    public std::unary_function<art::FileIndex::Element const &,
                               void> {
  public:
    EventIDIndexBuilder(art::MixOpBase::EventIDIndex &index);
    result_type operator()(argument_type bIDs) const;
  private:
    art::MixOpBase::EventIDIndex &index_;
  };

  boost::regex const & modeRegex() {
    static boost::regex r("^seq", boost::regex_constants::icase);
    return r;
  }
}

inline
EventIDIndexBuilder::EventIDIndexBuilder(art::MixOpBase::EventIDIndex &index)
  :
  index_(index)
{
  index_.clear();
}

EventIDIndexBuilder::result_type
EventIDIndexBuilder::operator()(argument_type element) const {
  if (element.getEntryType() == art::FileIndex::kEvent) {
    index_[element.entry_] = element.eventID_;
  }
}

art::MixHelper::MixHelper(fhicl::ParameterSet const &pset,
                          ProducerBase &producesProvider)
  :
  producesProvider_(producesProvider),
  filenames_(pset.get<std::vector<std::string> >("filenames")),
  mixOps_(),
  ptrRemapper_(),
  currentFilename_(filenames_.begin()),
  readMode_(boost::regex_search(pset.get<std::string>("readMode", "sequential"),
                                modeRegex())?SEQUENTIAL:RANDOM),
  coverageFraction_(pset.get<double>("coverageFraction", 100.0)),
  nEventsRead_(0),
  ffVersion_(),
  ptpBuilder_(),
  dist_(ServiceHandle<RandomNumberGenerator>()->getEngine()),
  eventIDIndex_(),
  currentFile_(),
  currentMetaDataTree_(),
  currentEventTree_(),
  dataBranches_()
{
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

  FileIndex fileIndex;
  FileIndex *fileIndex_p = &fileIndex;
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

  // Prepare to read EventHistory tree.
  TTree* ehTree = 
    dynamic_cast<TTree*>(currentFile_->
                         Get(rootNames::eventHistoryTreeName().c_str()));
  if (ehTree == 0) {
    throw Exception(errors::FileReadError)
      << "Unable to read event history tree from secondary event stream file "
      << *currentFilename_
      << ".\n";
  }

  buildEventIDIndex(fileIndex);

  ProdToProdMapBuilder::BranchIDTransMap transMap;
  buildBranchIDTransMap(transMap);
  ptpBuilder_.prepareTranslationTables(transMap, branchIDLists, ehTree);
}

void
art::MixHelper::buildEventIDIndex(FileIndex const &fileIndex) {
  std::for_each(fileIndex.begin(),
                fileIndex.end(),
                EventIDIndexBuilder(eventIDIndex_));
}

void
art::MixHelper::postRegistrationInit() {
  // Open and read the first file to read branch information.
  openAndReadMetaData(*currentFilename_);
}

void
art::MixHelper::mixAndPut(size_t nSecondaries, Event &e) {
  // Populate the remapper in case we need to remap any Ptrs.
  ptpBuilder_.populateRemapper(ptrRemapper_, e);

  // Decide which events we're reading and prime the event tree cache.
  MixOpBase::EntryNumberSequence enSeq;
  MixOpBase::EventIDSequence eIDseq;
  generateEventSequence(nSecondaries, enSeq, eIDseq);

  // Do the branch-wise read, mix and put.
  cet::for_all(mixOps_,
               std::bind(&MixHelper::mixAndPutOne,
                         this,
                         _1,
                         std::cref(enSeq),
                         std::cref(eIDseq),
                         std::ref(e)));
}

void
art::MixHelper::mixAndPutOne(boost::shared_ptr<MixOpBase> op,
                             MixOpBase::EntryNumberSequence const &enSeq,
                             MixOpBase::EventIDSequence const &eIDseq,
                             Event &e) {
  op->readFromFile(currentEventTree_, enSeq);
  op->mixAndPut(e, ptrRemapper_, eIDseq);
}

bool
art::MixHelper::
generateEventSequence(size_t nSecondaries,
                      MixOpBase::EntryNumberSequence const &enSeq,
                      MixOpBase::EventIDSequence const &eIDseq) {
  if (readMode_ == SEQUENTIAL) {
  } else { // RANDOM
  }
  return false;
}

void
art::MixHelper::buildBranchIDTransMap(ProdToProdMapBuilder::BranchIDTransMap &transMap) {
  for(MixOpList::const_iterator
        i = mixOps_.begin(),
        e = mixOps_.end();
      i != e;
      ++i) {
    transMap[(*i)->incomingBranchID(dataBranches_)] =
      (*i)->outgoingBranchID();
  }
}
