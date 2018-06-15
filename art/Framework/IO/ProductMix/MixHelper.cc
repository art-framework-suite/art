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
#include <random>
#include <regex>
#include <unordered_set>

#include "Rtypes.h"

using namespace std::string_literals;

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
    EventIDLookup(art::EventIDIndex const& index) : index_{index} {}

    result_type
    operator()(argument_type const entry) const
    {
      auto i = index_.find(entry);
      if (i == cend(index_)) {
        throw art::Exception(art::errors::LogicError)
          << "MixHelper could not find entry number " << entry
          << " in its own lookup table.\n";
      }
      return i->second;
    }

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

  double
  initCoverageFraction(double fraction)
  {
    if (fraction > (1 + std::numeric_limits<double>::epsilon())) {
      mf::LogWarning("Configuration")
        << "coverageFraction > 1: treating as a percentage.\n";
      fraction /= 100.0;
    }
    return fraction;
  }

} // namespace

art::MixHelper::MixHelper(fhicl::ParameterSet const& pset,
                          std::string const& moduleLabel,
                          Modifier& producesProvider)
  : detail::EngineCreator{moduleLabel, art::ScheduleID::first()}
  , producesProvider_{producesProvider}
  , filenames_{pset.get<std::vector<std::string>>("fileNames", {})}
  , compactMissingProducts_{pset.get<bool>("compactMissingProducts", false)}
  , fileIter_{filenames_.begin()}
  , readMode_{initReadMode_(pset.get<std::string>("readMode", "sequential"))}
  , coverageFraction_{initCoverageFraction(
      pset.get<double>("coverageFraction", 1.0))}
  , canWrapFiles_{pset.get<bool>("wrapFiles", false)}
  , engine_{initEngine_(pset.get<long>("seed", -1), readMode_)}
  , dist_{initDist_(engine_)}
{}

art::MixHelper::MixHelper(Config const& config,
                          std::string const& moduleLabel,
                          Modifier& producesProvider)
  : detail::EngineCreator{moduleLabel, art::ScheduleID::first()}
  , producesProvider_{producesProvider}
  , filenames_{config.filenames()}
  , compactMissingProducts_{config.compactMissingProducts()}
  , fileIter_{filenames_.begin()}
  , readMode_{initReadMode_(config.readMode())}
  , coverageFraction_{initCoverageFraction(config.coverageFraction())}
  , canWrapFiles_{config.wrapFiles()}
  , engine_{initEngine_(config.seed(), readMode_)}
  , dist_{initDist_(engine_)}
{}

std::ostream&
art::operator<<(std::ostream& os, MixHelper::Mode const mode)
{
  switch (mode) {
    case MixHelper::Mode::SEQUENTIAL:
      return os << "SEQUENTIAL";
    case MixHelper::Mode::RANDOM_REPLACE:
      return os << "RANDOM_REPLACE";
    case MixHelper::Mode::RANDOM_LIM_REPLACE:
      return os << "RANDOM_LIM_REPLACE";
    case MixHelper::Mode::RANDOM_NO_REPLACE:
      return os << "RANDOM_NO_REPLACE";
    case MixHelper::Mode::UNKNOWN:
      return os << "UNKNOWN";
      // No default so compiler can warn.
  }
  return os;
}

void
art::MixHelper::registerSecondaryFileNameProvider(ProviderFunc_ func)
{
  if (!filenames_.empty()) {
    throw Exception{errors::Configuration}
      << "Provision of a secondary file name provider is incompatible"
      << " with a\nnon-empty fileNames parameter to the mix filter.\n";
  }
  providerFunc_ = func;
}

art::MixHelper::base_engine_t&
art::MixHelper::createEngine(seed_t const seed)
{
  if (engine_ && consistentRequest_("HepJamesRandom", ""s)) {
    return *engine_;
  }
  return detail::EngineCreator::createEngine(seed);
}

art::MixHelper::base_engine_t&
art::MixHelper::createEngine(seed_t const seed,
                             std::string const& kind_of_engine_to_make)
{
  if (engine_ && consistentRequest_(kind_of_engine_to_make, ""s)) {
    return *engine_;
  }
  return detail::EngineCreator::createEngine(seed, kind_of_engine_to_make);
}

art::MixHelper::base_engine_t&
art::MixHelper::createEngine(seed_t const seed,
                             std::string const& kind_of_engine_to_make,
                             label_t const& engine_label)
{
  if (engine_ && consistentRequest_(kind_of_engine_to_make, engine_label)) {
    return *engine_;
  }
  return detail::EngineCreator::createEngine(
    seed, kind_of_engine_to_make, engine_label);
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
  if (over_threshold || (!ffVersion_.isValid())) {
    if (openNextFile_()) {
      return generateEventSequence(nSecondaries, enSeq, eIDseq);
    } else {
      return false;
    }
  }
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
      auto i = shuffledSequence_.cbegin() + nEventsReadThisFile_;
      enSeq.assign(i, i + nSecondaries);
    } break;
    default:
      throw Exception(errors::LogicError)
        << "Unrecognized read mode " << static_cast<int>(readMode_)
        << ". Contact the art developers.\n";
  }
  cet::transform_all(
    enSeq, back_inserter(eIDseq), EventIDLookup{eventIDIndex_});
  return true;
}

void
art::MixHelper::generateEventAuxiliarySequence(EntryNumberSequence const& enseq,
                                               EventAuxiliarySequence& auxseq)
{
  input::RootMutexSentry sentry;
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
    // Note: Root will overwrite the old event auxiliary with the new
    //       one.
    input::getEntry(auxBranch, entry);
    // Note: We are intentionally making a copy here of the fetched
    //       event auxiliary!
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

  // Create required info only if we're likely to need it.
  EntryNumberSequence subRunEntries;
  EntryNumberSequence runEntries;
  if (haveSubRunMixOps_) {
    subRunEntries.reserve(eIDseq.size());
    for (auto const& eID : eIDseq) {
      auto const it = currentFileIndex_.findPosition(eID.subRunID(), true);
      if (it != std::cend(currentFileIndex_)) {
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
      if (it != std::cend(currentFileIndex_)) {
        runEntries.emplace_back(it->entry_);
      } else {
        throw Exception(errors::NotFound, "NO_RUN")
          << "- Unable to find an entry in the Run tree corresponding to "
             "event ID "
          << eID << " in secondary mixing input file "
          << currentFile_->GetName() << ".\n";
      }
    }
  }

  // Dummy remapper in case we need it.
  static PtrRemapper const nopRemapper;

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
    // FIXME: threading: This is not thread-safe!!!
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
  auto transMap = buildProductIDTransMap_(mixOps_);
  ptpBuilder_.prepareTranslationTables(transMap);
  if (readMode_ == Mode::RANDOM_NO_REPLACE) {
    // Prepare shuffled event sequence.
    shuffledSequence_.resize(static_cast<size_t>(nEventsInFile_));
    std::iota(shuffledSequence_.begin(), shuffledSequence_.end(), 0);
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(shuffledSequence_.begin(), shuffledSequence_.end(), g);
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

art::ProdToProdMapBuilder::ProductIDTransMap
art::MixHelper::buildProductIDTransMap_(MixOpList& mixOps)
{
  ProdToProdMapBuilder::ProductIDTransMap transMap;
  for (auto& mixOp : mixOps) {
    auto const bt = mixOp->branchType();
    mixOp->initializeBranchInfo(dataBranches_[bt]);
    if (bt != InEvent)
      continue;
    transMap[mixOp->incomingProductID()] = mixOp->outgoingProductID();
  }
  return transMap;
}

bool
art::MixHelper::consistentRequest_(std::string const& kind_of_engine_to_make,
                                   label_t const& engine_label) const
{
  if (kind_of_engine_to_make == "HepJamesRandom"s && engine_label.empty()) {
    mf::LogInfo{"RANDOM"} << "A random number engine has already been created "
                             "since the read mode is "
                          << readMode_ << '.';
    return true;
  }
  throw Exception{errors::Configuration,
                  "An error occurred while creating a random number engine "
                  "within a MixFilter detail class.\n"}
    << "A random number engine with an empty label has already been created "
       "with an engine type of HepJamesRandom.\n"
    << "If you would like to use a different engine type, please supply a "
       "different engine label.\n";
}

cet::exempt_ptr<art::MixHelper::base_engine_t>
art::MixHelper::initEngine_(seed_t const seed, Mode const readMode)
{
  using namespace art;
  if (readMode > MixHelper::Mode::SEQUENTIAL) {
    if (ServiceRegistry::isAvailable<RandomNumberGenerator>()) {
      return cet::make_exempt_ptr(&detail::EngineCreator::createEngine(seed));
    } else {
      throw Exception{errors::Configuration, "MixHelper"}
        << "Random event mixing selected but RandomNumberGenerator service "
           "not loaded.\n"
        << "Ensure service is loaded with: \n"
        << "services.RandomNumberGenerator: {}\n";
    }
  }
  return nullptr;
}

std::unique_ptr<CLHEP::RandFlat>
art::MixHelper::initDist_(cet::exempt_ptr<base_engine_t> const engine) const
{
  std::unique_ptr<CLHEP::RandFlat> result{nullptr};
  if (engine) {
    result = std::make_unique<CLHEP::RandFlat>(*engine);
  }
  return result;
}
