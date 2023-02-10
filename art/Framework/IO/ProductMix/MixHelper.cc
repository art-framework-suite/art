#include "art/Framework/IO/ProductMix/MixHelper.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "cetlib/container_algorithms.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "range/v3/view.hpp"

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <numeric>
#include <ostream>
#include <random>
#include <regex>
#include <unordered_set>

using namespace ::ranges;
using namespace std::string_literals;

namespace {
  bool
  event_entries(art::FileIndex::Element const& element)
  {
    return element.getEntryType() == art::FileIndex::kEvent;
  }

  bool
  only_events(std::unique_ptr<art::MixOpBase> const& mixOp)
  {
    return mixOp->branchType() == art::InEvent;
  }

  art::EventIDIndex
  buildEventIDIndex(art::FileIndex const& fileIndex)
  {
    art::EventIDIndex result;
    for (auto const& element : fileIndex | views::filter(event_entries)) {
      result.try_emplace(element.entry, element.eventID);
    }
    return result;
  }

  art::ProdToProdMapBuilder::ProductIDTransMap
  buildProductIDTransMap(art::MixOpList const& mixOps)
  {
    art::ProdToProdMapBuilder::ProductIDTransMap transMap;
    for (auto const& mixOp : mixOps | views::filter(only_events)) {
      transMap[mixOp->incomingProductID()] = mixOp->outgoingProductID();
    }
    return transMap;
  }

  class EventIDLookup {
  public:
    explicit EventIDLookup(art::EventIDIndex const& index) : index_{index} {}

    art::EventID
    operator()(art::FileIndex::EntryNumber_t const entry) const
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

  double
  initCoverageFraction(double fraction)
  {
    if (fraction > (1 + std::numeric_limits<double>::epsilon())) {
      mf::LogWarning("Configuration")
        << "coverageFraction > 1: treating as a percentage.";
      fraction /= 100.0;
    }
    return fraction;
  }

} // namespace

art::MixHelper::MixHelper(fhicl::ParameterSet const& pset,
                          std::string const& moduleLabel,
                          ProducesCollector& collector,
                          std::unique_ptr<MixIOPolicy> ioHandle)
  : EngineCreator{moduleLabel, ScheduleID::first()}
  , collector_{collector}
  , moduleLabel_{moduleLabel}
  , filenames_{pset.get<std::vector<std::string>>("fileNames", {})}
  , compactMissingProducts_{pset.get<bool>("compactMissingProducts", false)}
  , fileIter_{filenames_.begin()}
  , readMode_{initReadMode_(pset.get<std::string>("readMode", "sequential"))}
  , coverageFraction_{initCoverageFraction(
      pset.get<double>("coverageFraction", 1.0))}
  , canWrapFiles_{pset.get<bool>("wrapFiles", false)}
  , engine_{initEngine_(pset.get<long>("seed", -1), readMode_)}
  , dist_{initDist_(engine_)}
  , ioHandle_{std::move(ioHandle)}
{}

art::MixHelper::MixHelper(Config const& config,
                          std::string const& moduleLabel,
                          ProducesCollector& collector,
                          std::unique_ptr<MixIOPolicy> ioHandle)
  : EngineCreator{moduleLabel, ScheduleID::first()}
  , collector_{collector}
  , moduleLabel_{moduleLabel}
  , filenames_{config.filenames()}
  , compactMissingProducts_{config.compactMissingProducts()}
  , fileIter_{filenames_.begin()}
  , readMode_{initReadMode_(config.readMode())}
  , coverageFraction_{initCoverageFraction(config.coverageFraction())}
  , canWrapFiles_{config.wrapFiles()}
  , engine_{initEngine_(config.seed(), readMode_)}
  , dist_{initDist_(engine_)}
  , ioHandle_{std::move(ioHandle)}
{}

art::MixHelper::~MixHelper() = default;

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
  if (engine_ &&
      consistentRequest_(
        ServiceHandle<RandomNumberGenerator const>()->defaultEngineKind(),
        ""s)) {
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
  if (not ioHandle_->fileOpen() and not openNextFile_()) {
    return false;
  }

  auto const nEventsInFile = ioHandle_->nEventsInFile();
  bool const over_threshold =
    (readMode_ == Mode::SEQUENTIAL || readMode_ == Mode::RANDOM_NO_REPLACE) ?
      ((nEventsReadThisFile_ + nSecondaries) > nEventsInFile) :
      ((nEventsReadThisFile_ + nSecondaries) >
       (nEventsInFile * coverageFraction_));
  if (over_threshold) {
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
    enSeq.resize(nSecondaries);
    std::iota(begin(enSeq), end(enSeq), nEventsReadThisFile_);
    break;
  case Mode::RANDOM_REPLACE:
    std::generate_n(
      std::back_inserter(enSeq), nSecondaries, [this, nEventsInFile] {
        return dist_.get()->fireInt(nEventsInFile);
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
        [this, nEventsInFile] { return dist_.get()->fireInt(nEventsInFile); });
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

art::EventAuxiliarySequence
art::MixHelper::generateEventAuxiliarySequence(EntryNumberSequence const& enSeq)
{
  return ioHandle_->generateEventAuxiliarySequence(enSeq);
}

namespace {
  art::PtrRemapper const nopRemapper{};
}

void
art::MixHelper::mixAndPut(EntryNumberSequence const& eventEntries,
                          EventIDSequence const& eIDseq,
                          Event& e)
{
  // Create required info only if we're likely to need it.
  EntryNumberSequence subRunEntries;
  EntryNumberSequence runEntries;
  auto const& fileIndex = ioHandle_->fileIndex();
  if (haveSubRunMixOps_) {
    subRunEntries.reserve(eIDseq.size());
    for (auto const& eID : eIDseq) {
      auto const it = fileIndex.findPosition(eID.subRunID(), true);
      if (it == std::cend(fileIndex)) {
        throw Exception(errors::NotFound, "NO_SUBRUN")
          << "- Unable to find an entry in the SubRun tree corresponding to "
             "event ID "
          << eID << " in secondary mixing input file " << *fileIter_ << ".\n";
      }
      subRunEntries.emplace_back(it->entry);
    }
  }
  if (haveRunMixOps_) {
    runEntries.reserve(eIDseq.size());
    for (auto const& eID : eIDseq) {
      auto const it = fileIndex.findPosition(eID.runID(), true);
      if (it == std::cend(fileIndex)) {
        throw Exception(errors::NotFound, "NO_RUN")
          << "- Unable to find an entry in the Run tree corresponding to "
             "event ID "
          << eID << " in secondary mixing input file " << *fileIter_ << ".\n";
      }
      runEntries.emplace_back(it->entry);
    }
  }

  // Populate the remapper in case we need to remap any Ptrs.
  ptrRemapper_ = ptpBuilder_.getRemapper(e);

  // Do the branch-wise read, mix and put.
  for (auto const& op : mixOps_) {
    switch (op->branchType()) {
    case InEvent: {
      auto const inProducts = ioHandle_->readFromFile(*op, eventEntries);
      op->mixAndPut(e, inProducts, ptrRemapper_);
      continue;
    }
    case InSubRun: {
      auto const inProducts = ioHandle_->readFromFile(*op, subRunEntries);
      // Ptrs not supported for subrun product mixing.
      op->mixAndPut(e, inProducts, nopRemapper);
      continue;
    }
    case InRun: {
      auto const inProducts = ioHandle_->readFromFile(*op, runEntries);
      // Ptrs not support for run product mixing.
      op->mixAndPut(e, inProducts, nopRemapper);
      continue;
    }
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
    if (ioHandle_->fileOpen()) { // Already seen one file.
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
  ioHandle_->openAndReadMetaData(filename, mixOps_);

  eventIDIndex_ = buildEventIDIndex(ioHandle_->fileIndex());
  auto transMap = buildProductIDTransMap(mixOps_);
  ptpBuilder_.prepareTranslationTables(transMap);

  if (readMode_ == Mode::RANDOM_NO_REPLACE) {
    // Prepare shuffled event sequence.
    shuffledSequence_.resize(ioHandle_->nEventsInFile());
    std::iota(shuffledSequence_.begin(), shuffledSequence_.end(), 0);
    std::random_device rd;
    std::mt19937 g{rd()};
    std::shuffle(shuffledSequence_.begin(), shuffledSequence_.end(), g);
  }

  return true;
}

bool
art::MixHelper::consistentRequest_(std::string const& kind_of_engine_to_make,
                                   label_t const& engine_label) const
{
  auto const& default_engine_kind =
    ServiceHandle<RandomNumberGenerator const>()->defaultEngineKind();
  if (kind_of_engine_to_make == default_engine_kind && engine_label.empty()) {
    mf::LogInfo{"RANDOM"} << "A random number engine has already been created "
                             "since the read mode is "
                          << readMode_ << '.';
    return true;
  }
  throw Exception{errors::Configuration,
                  "An error occurred while creating a random number engine "
                  "within a MixFilter detail class.\n"}
    << "A random number engine with an empty label has already been created "
       "with an engine type of "
    << default_engine_kind << ".\n"
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
    }
    throw Exception{errors::Configuration, "MixHelper"}
      << "Random event mixing selected but RandomNumberGenerator service "
         "not loaded.\n"
      << "Ensure service is loaded with: \n"
      << "services.RandomNumberGenerator: {}\n";
  }
  return nullptr;
}

std::unique_ptr<CLHEP::RandFlat>
art::MixHelper::initDist_(cet::exempt_ptr<base_engine_t> const engine) const
{
  return engine ? std::make_unique<CLHEP::RandFlat>(*engine) : nullptr;
}
