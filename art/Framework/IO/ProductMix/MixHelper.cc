#include "art/Framework/IO/ProductMix/MixHelper.h"

#include "Rtypes.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/setMetaDataBranchAddress.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Persistency/Provenance/History.h"
#include "cpp0x/regex"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cassert>
#include <functional>
#include <limits>

namespace {
  class EventIDIndexBuilder :
    public std::unary_function<art::FileIndex::Element const &,
                               void> {
  public:
    EventIDIndexBuilder(art::EventIDIndex &index);
    result_type operator()(argument_type bIDs) const;
  private:
    art::EventIDIndex &index_;
  };

  class EventIDLookup :
    public std::unary_function<Long64_t, art::EventID> {
  public:
    EventIDLookup(art::EventIDIndex const &index);
    result_type operator()(argument_type bIDs) const;
  private:
    art::EventIDIndex const &index_;
  };


  std::regex const & modeRegex() {
    static std::regex r("^seq", std::regex_constants::icase);
    return r;
  }
}  // namespace

inline
EventIDIndexBuilder::EventIDIndexBuilder(art::EventIDIndex &index)
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

inline
EventIDLookup::EventIDLookup(art::EventIDIndex const &index)
  :
  index_(index)
{}

EventIDLookup::result_type
EventIDLookup::operator()(argument_type element) const {
  art::EventIDIndex::const_iterator i = index_.find(element);
  if (i == index_.end()) {
    throw art::Exception(art::errors::LogicError)
      << "MixHelper could not find entry number "
      << element
      << " in its own lookup table.\n";
  }
  return i->second;
}

art::MixHelper::MixHelper(fhicl::ParameterSet const &pset,
                          ProducerBase &producesProvider)
  :
  producesProvider_(producesProvider),
  filenames_(pset.get<std::vector<std::string> >("fileNames")),
  mixOps_(),
  ptrRemapper_(),
  currentFilename_(filenames_.begin()),
  readMode_(std::regex_search(pset.get<std::string>("readMode", "sequential"),
                              modeRegex())?SEQUENTIAL:RANDOM),
  coverageFraction_(pset.get<double>("coverageFraction", 1.0)),
  nEventsRead_(0),
  nEventsInFile_(0),
  ffVersion_(),
  ptpBuilder_(),
  dist_(ServiceHandle<RandomNumberGenerator>()->getEngine()),
  eventIDIndex_(),
  currentFile_(),
  currentMetaDataTree_(),
  currentEventTree_(),
  dataBranches_()
{
  if (coverageFraction_ > (1 + std::numeric_limits<double>::epsilon())) {
    mf::LogWarning("Configuration")
      << "coverageFraction > 1: treating as a percentage.\n";
    coverageFraction_ /= 100.0;
  }
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
  nEventsInFile_ = currentEventTree_->GetEntries();

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

bool
art::MixHelper::
generateEventSequence(size_t nSecondaries,
                      EntryNumberSequence &enSeq,
                      EventIDSequence &eIDseq) {
  assert(enSeq.empty());
  assert(eIDseq.empty());
  bool over_threshold = (readMode_ == SEQUENTIAL)?
    ((nEventsRead_ + nSecondaries) > nEventsInFile_):
    ((nEventsRead_ + nSecondaries) > (nEventsInFile_ * coverageFraction_));
  if (over_threshold) {
    if (openNextFile()) {
      return generateEventSequence(nSecondaries, enSeq, eIDseq);
    } else {
      return false;
    }
  }
  if (readMode_ == SEQUENTIAL) {
    for (size_t
           i = nEventsRead_,
           end = nEventsRead_ + nSecondaries;
         i < end;
         ++i) {
      enSeq.push_back(i);
    }
  } else { // RANDOM
    std::generate_n(std::back_inserter(enSeq),
                    nSecondaries,
                    std::bind(static_cast<long (CLHEP::RandFlat::*) (long)>
                              (&CLHEP::RandFlat::fireInt), // Resolve overload.
                              &dist_, nEventsInFile_));
    std::sort(enSeq.begin(), enSeq.end());
  }
  std::transform(enSeq.begin(),
                 enSeq.end(),
                 std::back_inserter(eIDseq),
                 EventIDLookup(eventIDIndex_));
  return true;
}

void
art::MixHelper::mixAndPut(EntryNumberSequence const &enSeq,
                          Event &e) {
  // Populate the remapper in case we need to remap any Ptrs.
  ptpBuilder_.populateRemapper(ptrRemapper_, e);

  // Do the branch-wise read, mix and put.
  cet::for_all(mixOps_,
               std::bind(&MixHelper::mixAndPutOne,
                         this,
                         _1,
                         enSeq,
                         std::ref(e)));

  nEventsRead_ += enSeq.size();
}

void
art::MixHelper::mixAndPutOne(std::shared_ptr<MixOpBase> op,
                             EntryNumberSequence const &enSeq,
                             Event &e) {
  op->readFromFile(enSeq);
  op->mixAndPut(e, ptrRemapper_);
}

bool
art::MixHelper::openNextFile() {
  if (++currentFilename_ == filenames_.end()) {
    return false;
  }
  openAndReadMetaData(*currentFilename_);
  return true;
}

void
art::MixHelper::buildBranchIDTransMap(ProdToProdMapBuilder::BranchIDTransMap &transMap) {
  for(MixOpList::const_iterator
        i = mixOps_.begin(),
        e = mixOps_.end();
      i != e;
      ++i) {
    (*i)->initializeBranchInfo(dataBranches_);
#if ART_DEBUG_PTRREMAPPER
    std::cerr << "BranchIDTransMap: "
              << std::hex
              << std::setfill('0')
              << std::setw(8)
              << (*i)->incomingBranchID()
              << " -> "
              << std::setw(8)
              << (*i)->outgoingBranchID()
              << std::dec
              << ".\n";
#endif
    transMap[(*i)->incomingBranchID()] =
      (*i)->outgoingBranchID();
  }
}
