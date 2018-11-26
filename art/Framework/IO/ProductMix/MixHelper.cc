#include "art/Framework/IO/ProductMix/MixHelper.h"

#include "art/Framework/IO/Root/GetFileFormatEra.h"
#include "art/Framework/IO/Root/detail/readFileIndex.h"
#include "art/Framework/IO/Root/detail/readMetadata.h"

#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas_root_io/Streamers/ProductIDStreamer.h"
#include "cetlib/container_algorithms.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <numeric>
#include <ostream>
#include <regex>
#include <unordered_set>

#include "Rtypes.h"

namespace {

  art::EventIDIndex
  buildEventIDIndex(art::FileIndex const& fileIndex)
  {
    art::EventIDIndex result;
    for (auto const& element : fileIndex) {
      if (element.getEntryType() != art::FileIndex::kEvent)
        continue;
      result.emplace(element.entry_, element.eventID_);
    }
    return result;
  }

  class EventIDLookup : public std::unary_function<Long64_t, art::EventID> {
  public:
    EventIDLookup(art::EventIDIndex const& index);
    result_type operator()(argument_type entry) const;

  private:
    art::EventIDIndex const& index_;
  };

  std::array<cet::exempt_ptr<TTree>, art::NumBranchTypes>
  initDataTrees(cet::value_ptr<TFile> const& currentFile)
  {
    std::array<cet::exempt_ptr<TTree>, art::NumBranchTypes> result;
    for (auto bt = 0; bt != art::NumBranchTypes; ++bt) {
      result[bt].reset(static_cast<TTree*>(currentFile->Get(
        art::rootNames::dataTreeName(static_cast<art::BranchType>(bt))
          .c_str())));
      if (result[bt].get() == nullptr and bt != art::InResults) {
        throw art::Exception(art::errors::FileReadError)
          << "Unable to read event tree from secondary event stream file "
          << currentFile->GetName() << ".\n";
      }
    }
    return result;
  }
} // namespace

inline EventIDLookup::EventIDLookup(art::EventIDIndex const& index)
  : index_(index)
{}

EventIDLookup::result_type
EventIDLookup::operator()(argument_type entry) const
{
  auto i = index_.find(entry);
  if (i == cend(index_)) {
    throw art::Exception(art::errors::LogicError)
      << "MixHelper could not find entry number " << entry
      << " in its own lookup table.\n";
  }
  return i->second;
}

namespace {
  double
  initCoverageFraction(fhicl::ParameterSet const& pset)
  {
    auto result = pset.get<double>("coverageFraction", 1.0);
    if (result > (1 + std::numeric_limits<double>::epsilon())) {
      mf::LogWarning("Configuration")
        << "coverageFraction > 1: treating as a percentage.\n";
      result /= 100.0;
    }
    return result;
  }

  std::unique_ptr<CLHEP::RandFlat>
  initDist(art::MixHelper::Mode readMode)
  {
    using namespace art;
    std::unique_ptr<CLHEP::RandFlat> result;
    if (readMode > MixHelper::Mode::SEQUENTIAL) {
      if (ServiceRegistry::isAvailable<RandomNumberGenerator>()) {
        result = std::make_unique<CLHEP::RandFlat>(
          ServiceHandle<RandomNumberGenerator> {}->getEngine());
      } else {
        throw Exception(errors::Configuration, "MixHelper")
          << "Random event mixing selected but RandomNumberGenerator service "
             "not loaded.\n"
          << "Ensure service is loaded with: \n"
          << "services.RandomNumberGenerator: {}\n";
      }
    }
    return result;
  }
}

art::MixHelper::MixHelper(fhicl::ParameterSet const& pset,
                          ProducerBase& producesProvider)
  : producesProvider_{producesProvider}
  , filenames_{pset.get<std::vector<std::string>>("fileNames", {})}
  , compactMissingProducts_{pset.get<bool>("compactMissingProducts", false)}
  , fileIter_{filenames_.begin()}
  , readMode_{initReadMode_(pset.get<std::string>("readMode", "sequential"))}
  , coverageFraction_{initCoverageFraction(pset)}
  , canWrapFiles_{pset.get<bool>("wrapFiles", false)}
  , dist_{initDist(readMode_)}
{}

void
art::MixHelper::registerSecondaryFileNameProvider(ProviderFunc_ func)
{
  if (!filenames_.empty()) {
    throw Exception(errors::Configuration)
      << "Provision of a secondary file name provider is incompatible"
      << " with a\nnon-empty fileNames parameter to the mix filter.\n";
  }
  providerFunc_ = func;
}

bool
art::MixHelper::generateEventSequence(size_t const nSecondaries,
                                      EntryNumberSequence& enSeq,
                                      EventIDSequence& eIDseq)
{
  assert(enSeq.empty());
  assert(eIDseq.empty());
  assert(nEventsInFile_ >= 0);

  bool over_threshold =
    (readMode_ == Mode::SEQUENTIAL || readMode_ == Mode::RANDOM_NO_REPLACE) ?
      ((nEventsReadThisFile_ + nSecondaries) >
       static_cast<size_t>(nEventsInFile_)) :
      ((nEventsReadThisFile_ + nSecondaries) >
       (nEventsInFile_ * coverageFraction_));
  if (over_threshold || !ffVersion_.isValid()) {
    if (!providerFunc_) {
      ++nOpensOverThreshold_;
      if (nOpensOverThreshold_ > filenames_.size()) {
        throw Exception{errors::UnimplementedFeature,
                        "An error occurred while preparing product-mixing for "
                        "the current event.\n"}
          << "The number of requested secondaries (" << nSecondaries
          << ") exceeds the number of events in any\n"
          << "of the files specified for product mixing.  For a read mode of '"
          << readMode_ << "',\n"
          << "the framework does not currently allow product-mixing to span "
             "multiple secondary\n"
          << "input files for a given event.  Please contact artists@fnal.gov "
             "for more information.\n";
      }
    }
    if (openNextFile_()) {
      return generateEventSequence(nSecondaries, enSeq, eIDseq);
    } else {
      return false;
    }
  }

  nOpensOverThreshold_ = {};
  switch (readMode_) {
    case Mode::SEQUENTIAL:
      enSeq.reserve(nSecondaries);
      for (size_t i = nEventsReadThisFile_,
                  end = nEventsReadThisFile_ + nSecondaries;
           i < end;
           ++i) {
        enSeq.push_back(i);
      }
      break;
    case Mode::RANDOM_REPLACE:
      std::generate_n(std::back_inserter(enSeq), nSecondaries, [this] {
        return dist_.get()->fireInt(nEventsInFile_);
      });
      std::sort(enSeq.begin(), enSeq.end());
      break;
    case Mode::RANDOM_LIM_REPLACE: {
      std::unordered_set<EntryNumberSequence::value_type>
        entries; // Guaranteed unique.
      while (entries.size() < nSecondaries) {
        std::generate_n(
          std::inserter(entries, entries.begin()),
          nSecondaries - entries.size(),
          [this] { return dist_.get()->fireInt(nEventsInFile_); });
      }
      enSeq.assign(cbegin(entries), cend(entries));
      std::sort(begin(enSeq), end(enSeq));
      // Since we need to sort at the end anyway, it's unclear whether
      // unordered_set is faster than set even though inserts are
      // approximately linear time. Since the complexity of the sort is
      // NlogN, we'd need a profile run for it all to come out in the
      // wash.
      assert(enSeq.size() == nSecondaries); // Should be true by construction.
    } break;
    case Mode::RANDOM_NO_REPLACE: {
      auto i = cbegin(shuffledSequence_) + nEventsReadThisFile_;
      enSeq.assign(i, i + nSecondaries);
    } break;
    default:
      throw Exception(errors::LogicError)
        << "Unrecognized read mode " << static_cast<int>(readMode_)
        << ". Contact the art developers.\n";
  }
  cet::transform_all(
    enSeq, std::back_inserter(eIDseq), EventIDLookup{eventIDIndex_});
  return true;
}

void
art::MixHelper::generateEventAuxiliarySequence(EntryNumberSequence const& enseq,
                                               EventAuxiliarySequence& auxseq)
{
  auto const eventTree = currentDataTrees_[InEvent];
  auto auxBranch =
    eventTree->GetBranch(BranchTypeToAuxiliaryBranchName(InEvent).c_str());
  auto aux = std::make_unique<EventAuxiliary>();
  auto pAux = aux.get();
  auxBranch->SetAddress(&pAux);
  for (auto const entry : enseq) {
    auto err = eventTree->LoadTree(entry);
    if (err == -2) {
      // FIXME: Throw an error here, taking care to disconnect the
      // branch from the i/o buffer.
      // FIXME: -2 means entry number too big.
    }
    // Note: Root will overwrite the old event
    //       auxiliary with the new one.
    input::getEntry(auxBranch, entry);
    // Note: We are intentionally making a copy here
    //       of the fetched event auxiliary!
    auxseq.push_back(*pAux);
  }
  // Disconnect the branch from the i/o buffer.
  auxBranch->SetAddress(nullptr);
}

void
art::MixHelper::mixAndPut(EntryNumberSequence const& eventEntries,
                          EventIDSequence const& eIDseq,
                          Event& e)
{
  // Populate the remapper in case we need to remap any Ptrs.
  ptpBuilder_.populateRemapper(ptrRemapper_, e);
  // Dummy remapper in case we need it.
  static PtrRemapper const nopRemapper;
  // Create required info only if we're likely to need it:
  EntryNumberSequence subRunEntries;
  EntryNumberSequence runEntries;
  if (haveSubRunMixOps_) {
    subRunEntries.reserve(eIDseq.size());
    for (auto const& eID : eIDseq) {
      auto const it = currentFileIndex_.findPosition(eID.subRunID(), true);
      if (it != currentFileIndex_.cend()) {
        subRunEntries.emplace_back(it->entry_);
      } else {
        throw Exception(errors::NotFound, "NO_SUBRUN")
          << "- Unable to find an entry in the SubRun tree corresponding to "
             "event ID "
          << eID << " in secondary mixing input file "
          << currentFile_->GetName() << ".\n";
      }
    }
  }
  if (haveRunMixOps_) {
    runEntries.reserve(eIDseq.size());
    for (auto const& eID : eIDseq) {
      auto const it = currentFileIndex_.findPosition(eID.runID(), true);
      if (it != currentFileIndex_.cend()) {
        runEntries.emplace_back(it->entry_);
      } else {
        throw Exception(errors::NotFound, "NO_RUN")
          << "- Unable to find an entry in the Run tree corresponding to event "
             "ID "
          << eID << " in secondary mixing input file "
          << currentFile_->GetName() << ".\n";
      }
    }
  }
  // Do the branch-wise read, mix and put.
  for (auto const& op : mixOps_) {
    switch (op->branchType()) {
      case InEvent:
        op->readFromFile(eventEntries, branchIDLists_.get());
        op->mixAndPut(e, ptrRemapper_);
        break;
      case InSubRun:
        op->readFromFile(subRunEntries, nullptr);
        // No Ptrs in subrun products.
        op->mixAndPut(e, nopRemapper);
        break;
      case InRun:
        op->readFromFile(runEntries, nullptr);
        // No Ptrs in run products.
        op->mixAndPut(e, nopRemapper);
        break;
      default:
        throw Exception(errors::LogicError, "Unsupported BranchType")
          << "- MixHelper::mixAndPut() attempted to handle unsupported branch "
             "type "
          << op->branchType() << ".\n";
    }
  }
  nEventsReadThisFile_ += eventEntries.size();
  totalEventsRead_ += eventEntries.size();
}

void
art::MixHelper::setEventsToSkipFunction(std::function<size_t()> eventsToSkip)
{
  eventsToSkip_ = eventsToSkip;
}

std::ostream&
art::operator<<(std::ostream& os, MixHelper::Mode const m)
{
  switch (m) {
    case MixHelper::Mode::SEQUENTIAL:
      os << "SEQUENTIAL";
      break;
    case MixHelper::Mode::RANDOM_REPLACE:
      os << "RANDOM_REPLACE";
      break;
    case MixHelper::Mode::RANDOM_LIM_REPLACE:
      os << "RANDOM_LIM_REPLACE";
      break;
    case MixHelper::Mode::RANDOM_NO_REPLACE:
      os << "RANDOM_NO_REPLACE";
      break;
    case MixHelper::Mode::UNKNOWN:
      os << "UNKNOWN";
  }
  return os;
}

auto
art::MixHelper::initReadMode_(std::string const& mode) const -> Mode
{
  // These regexes must correspond by index to the valid Mode enumerator
  // values.
  static std::regex const robjs[4]{
    std::regex("^seq", std::regex_constants::icase),
    std::regex("^random(replace)?$", std::regex_constants::icase),
    std::regex("^randomlimreplace$", std::regex_constants::icase),
    std::regex("^randomnoreplace$", std::regex_constants::icase)};
  int i{0};
  for (auto const& r : robjs) {
    if (std::regex_search(mode, r)) {
      return Mode(i);
    } else {
      ++i;
    }
  }
  throw Exception(errors::Configuration)
    << "Unrecognized value of readMode parameter: \"" << mode
    << "\". Valid values are:\n"
    << "  sequential,\n"
    << "  randomReplace (random is accepted for reasons of legacy),\n"
    << "  randomLimReplace,\n"
    << "  randomNoReplace.\n";
}

void
art::MixHelper::openAndReadMetaData_(std::string filename)
{
  // Open file.
  try {
    currentFile_.reset(TFile::Open(filename.c_str()));
  }
  catch (std::exception const& e) {
    throw Exception(errors::FileOpenError, e.what())
      << "Unable to open specified secondary event stream file " << filename
      << ".\n";
  }
  if (!currentFile_ || currentFile_->IsZombie()) {
    throw Exception(errors::FileOpenError)
      << "Unable to open specified secondary event stream file " << filename
      << ".\n";
  }
  // Obtain meta data tree.
  currentMetaDataTree_.reset(static_cast<TTree*>(
    currentFile_->Get(rootNames::metaDataTreeName().c_str())));
  if (currentMetaDataTree_.get() == nullptr) {
    throw Exception(errors::FileReadError)
      << "Unable to read meta data tree from secondary event stream file "
      << filename << ".\n";
  }
  currentDataTrees_ = initDataTrees(currentFile_);
  nEventsInFile_ = currentDataTrees_[InEvent]->GetEntries();

  ffVersion_ =
    detail::readMetadata<FileFormatVersion>(currentMetaDataTree_.get());

  // Read file index
  FileIndex* fileIndexPtr = &currentFileIndex_;
  detail::readFileIndex(
    currentFile_.get(), currentMetaDataTree_.get(), fileIndexPtr);

  // To support files that contain BranchIDLists
  BranchIDLists branchIDLists{};
  if (detail::readMetadata(currentMetaDataTree_.get(), branchIDLists)) {
    branchIDLists_ = std::make_unique<BranchIDLists>(std::move(branchIDLists));
    configureProductIDStreamer(branchIDLists_.get());
  }

  // Check file format era.
  std::string const expected_era = getFileFormatEra();
  if (ffVersion_.era_ != expected_era) {
    throw Exception(errors::FileReadError)
      << "Can only read files written during the \"" << expected_era
      << "\" era: "
      << "Era of "
      << "\"" << filename << "\" was "
      << (ffVersion_.era_.empty() ? "not set" :
                                    ("set to \"" + ffVersion_.era_ + "\" "))
      << ".\n";
  }
  auto dbCount = 0;
  for (auto const tree : currentDataTrees_) {
    if (tree.get()) {
      dataBranches_[dbCount].reset(tree.get());
    }
    ++dbCount;
  }

  eventIDIndex_ = buildEventIDIndex(currentFileIndex_);
  ProdToProdMapBuilder::ProductIDTransMap transMap;
  buildProductIDTransMap_(transMap);
  ptpBuilder_.prepareTranslationTables(transMap);
  if (readMode_ == Mode::RANDOM_NO_REPLACE) {
    // Prepare shuffled event sequence.
    shuffledSequence_.resize(static_cast<size_t>(nEventsInFile_));
    std::iota(shuffledSequence_.begin(), shuffledSequence_.end(), 0);
    std::random_shuffle(shuffledSequence_.begin(),
                        shuffledSequence_.end(),
                        [this](EntryNumberSequence::difference_type const& n) {
                          return dist_.get()->fireInt(n);
                        });
  }
}

bool
art::MixHelper::openNextFile_()
{
  std::string filename;
  if (providerFunc_) {
    filename = providerFunc_();
    if (filename.empty()) {
      return false;
    }
  } else if (filenames_.empty()) {
    return false;
  } else {
    if (ffVersion_.isValid()) { // Already seen one file.
      ++fileIter_;
    }
    if (fileIter_ == filenames_.end()) {
      if (canWrapFiles_) {
        mf::LogWarning("MixingInputWrap")
          << "Wrapping around to initial input file for mixing after "
          << totalEventsRead_ << " secondary events read.";
        fileIter_ = filenames_.begin();
      } else {
        return false;
      }
    }
    filename = *fileIter_;
  }
  nEventsReadThisFile_ = (readMode_ == Mode::SEQUENTIAL && eventsToSkip_) ?
                           eventsToSkip_() :
                           0; // Reset for this file.
  openAndReadMetaData_(filename);
  return true;
}

void
art::MixHelper::buildProductIDTransMap_(
  ProdToProdMapBuilder::ProductIDTransMap& transMap)
{
  for (auto& mixOp : mixOps_) {
    auto const bt = mixOp->branchType();
    mixOp->initializeBranchInfo(dataBranches_[bt]);
    if (bt == InEvent) {
      transMap[mixOp->incomingProductID()] = mixOp->outgoingProductID();
    }
  }
}
