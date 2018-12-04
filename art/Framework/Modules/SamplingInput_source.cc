// ==============================================================
// SamplingInput
// ==============================================================

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/detail/issue_reports.h"
#include "art/Framework/Modules/SampledInfo.h"
#include "art/Framework/Modules/detail/DataSetSampler.h"
#include "art/Framework/Modules/detail/SamplingInputFile.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/HorizontalRule.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/DelegatedParameter.h"
#include "fhiclcpp/types/OptionalAtom.h"
#include "fhiclcpp/types/Sequence.h"

#include <iomanip>
#include <string>
#include <vector>

using namespace fhicl;
using namespace std::string_literals;

namespace art {

  class SamplingInput : public art::InputSource {
  public:
    struct Config {
      Atom<std::string> module_type{Name{"module_type"}};
      Atom<unsigned> maxEvents{Name{"maxEvents"}, 1u};
      Atom<bool> delayedReadEventProducts{Name{"delayedReadEventProducts"},
                                          true};
      Atom<bool> readParameterSets{Name{"readParameterSets"}, true};
      OptionalAtom<RunNumber_t> run{
        Name{"run"},
        Comment{
          "The specified run and subrun numbers are used for all primary "
          "events\n"
          "generated in a given art job.  If no values are specified, default\n"
          "values of 1 and 0 are chosen for the run and subrun, "
          "respectively."}};
      OptionalAtom<SubRunNumber_t> subRun{Name("subRun")};
      OptionalAtom<EventNumber_t> firstEvent{Name("firstEvent")};

      DelegatedParameter dataSets{
        Name{"dataSets"},
        Comment{
          "The value of the 'dataSets' parameter is a table of the form:\n\n"
          "  dataSets: {\n"
          "    <dataset name>: {\n"
          "      fileName: <string>\n"
          "      weight: <double>\n"
          "    }\n"
          "    ...\n"
          "  }\n\n"
          "where the '<dataset name>' parameter labels a particular\n"
          "dataset (e.g. 'signal'), the 'fileName' refers to the file\n"
          "that contains events of the dataset, and the 'weight'\n"
          "is a floating-point number used to determine the frequency\n"
          "with which events from this dataset are sampled, relative to\n"
          "the sum of weights across all datasets.\n\n"
          "The ellipsis indicates that multiple datasets can be configured."}};
      Atom<bool> summary{Name{"summary"}, false};

      struct KeysToIgnore {
        std::set<std::string>
        operator()() const
        {
          return {"module_label"};
        }
      };
    };

    using Parameters = WrappedTable<Config, Config::KeysToIgnore>;
    explicit SamplingInput(Parameters const& config,
                           art::InputSourceDescription& isd);

  private:
    input::ItemType nextItemType() override;
    std::unique_ptr<FileBlock> readFile() override;
    void closeFile() override;
    std::unique_ptr<RunPrincipal> readRun() override;
    std::unique_ptr<SubRunPrincipal> readSubRun(
      cet::exempt_ptr<RunPrincipal const> rp) override;
    std::unique_ptr<EventPrincipal> readEvent(
      cet::exempt_ptr<SubRunPrincipal const> srp) override;
    std::unique_ptr<RangeSetHandler> runRangeSetHandler() override;
    std::unique_ptr<RangeSetHandler> subRunRangeSetHandler() override;
    void doEndJob() override;

    art::ProcessConfiguration const pc_;
    art::MasterProductRegistry& preg_;
    art::RunID runID_;
    art::SubRunID subRunID_;
    art::EventID eventID_;
    bool inputFileSet_{false};
    bool runSet_{false};
    bool subRunSet_{false};
    unsigned const maxEvents_;
    unsigned generatedEvents_{};
    detail::DataSetSampler dataSetSampler_;
    std::map<std::string, unsigned> counts_;
    unsigned totalCounts_{};
    bool const summary_;
    bool const delayedReadEventProducts_;
    bool const readParameterSets_;
    std::map<std::string, detail::SamplingInputFile> files_;
    ProductDescriptions sampledInfoDescriptions_;
    ProductTable subRunPresentProducts_;
    ProductTable runPresentProducts_;
    cet::exempt_ptr<std::string const> currentDataset_{nullptr};
    input::EntryNumber currentEntryInCurrentDataset_{
      std::numeric_limits<input::EntryNumber>::max()};
  };
}

namespace {
  constexpr auto
  nullTimestamp()
  {
    return art::Timestamp{};
  }
}

art::SamplingInput::SamplingInput(Parameters const& config,
                                  art::InputSourceDescription& isd)
  : InputSource{isd.moduleDescription}
  , pc_{isd.moduleDescription.processConfiguration()}
  , preg_{isd.productRegistry}
  , maxEvents_{config().maxEvents()}
  , dataSetSampler_{config().dataSets.get<ParameterSet>()}
  , summary_{config().summary()}
  , delayedReadEventProducts_{config().delayedReadEventProducts()}
  , readParameterSets_{config().readParameterSets()}
{
  if (!readParameterSets_) {
    mf::LogWarning("PROVENANCE")
      << "Source parameter readParameterSets was set to false: parameter set "
         "provenance\n"
      << "will NOT be available in this or subsequent jobs using output from "
         "this job.\n"
      << "Check your experiment's policy on this issue to avoid future "
         "problems\n"
      << "with analysis reproducibility.";
  }

  RunNumber_t r{};
  bool const haveFirstRun = config().run(r);
  runID_ = haveFirstRun ? RunID{r} : RunID::firstRun();

  SubRunNumber_t sr{};
  bool const haveFirstSubRun = config().subRun(sr);
  subRunID_ =
    haveFirstSubRun ? SubRunID{runID_, sr} : SubRunID::firstSubRun(runID_);

  EventNumber_t event{};
  bool const haveFirstEvent = config().firstEvent(event);
  eventID_ =
    haveFirstEvent ? EventID{subRunID_, event} : EventID::firstEvent(subRunID_);

  {
    mf::LogInfo log{"SamplingInput"};
    log << "The following datasets have been configured for the SamplingInput "
           "source:\n\n";

    std::string const spaces(4, ' ');
    std::string const dataset_field{"Dataset"};
    std::size_t width_of_dataset_field{dataset_field.size()};
    cet::for_all(dataSetSampler_.datasets(),
                 [& w = width_of_dataset_field](auto const& dataset) {
                   w = std::max(w, dataset.size());
                 });

    std::string const specified_field{"Specified"};
    std::string const weight_field{"weight"};
    std::size_t const width_of_weight_field{specified_field.size()};

    std::string const expected_field{"Expected"};
    std::string const fraction_field{"fraction"};
    std::size_t const width_of_fraction_field{expected_field.size()};

    std::string const filename_field{"File name"};
    std::size_t width_of_filename_field{filename_field.size()};
    cet::for_all(dataSetSampler_.datasets(),
                 [& w = width_of_filename_field,
                  &sampler = dataSetSampler_](auto const& dataset) {
                   w = std::max(w, sampler.fileName(dataset).size());
                 });

    // Cap the width at 100 so the printout isn't ridiculous.
    auto const rule_width =
      std::min(static_cast<std::size_t>(100),
               width_of_dataset_field + 4 + width_of_weight_field + 4 +
                 width_of_fraction_field + 4 + width_of_filename_field);
    cet::HorizontalRule const rule{rule_width};

    log << std::string(width_of_dataset_field, ' ') << spaces << std::left
        << std::setw(width_of_weight_field) << specified_field << spaces
        << std::left << std::setw(width_of_fraction_field) << expected_field
        << '\n';
    log << std::setw(width_of_dataset_field) << std::right << dataset_field
        << spaces << std::setw(width_of_weight_field) << std::right
        << weight_field << spaces << std::setw(width_of_fraction_field)
        << std::right << fraction_field << spaces << std::left << filename_field
        << '\n';
    log << rule('-');

    for (auto const& name : dataSetSampler_.datasets()) {
      log << '\n'
          << std::setw(width_of_dataset_field) << std::left << name << spaces
          << std::setw(width_of_weight_field) << std::right
          << dataSetSampler_.weight(name) << spaces
          << std::setw(width_of_fraction_field) << std::right
          << dataSetSampler_.probability(name) << spaces << std::left
          << dataSetSampler_.fileName(name);
    }
  }

  // Register SampledRunInfo, SampledSubRunInfo, and SampledEventInfo data
  // products. N.B. Order is important!
  auto const emulated_module_name = "SamplingInput"s;
  sampledInfoDescriptions_.emplace_back(
    InEvent,
    TypeLabel{
      TypeID{typeid(SampledEventInfo)}, {}, false, emulated_module_name},
    isd.moduleDescription);
  assert(sampledInfoDescriptions_[InEvent].branchType() == InEvent);
  sampledInfoDescriptions_.emplace_back(
    InSubRun,
    TypeLabel{
      TypeID{typeid(SampledSubRunInfo)}, {}, false, emulated_module_name},
    isd.moduleDescription);
  assert(sampledInfoDescriptions_[InSubRun].branchType() == InSubRun);
  sampledInfoDescriptions_.emplace_back(
    InRun,
    TypeLabel{TypeID{typeid(SampledRunInfo)}, {}, false, emulated_module_name},
    isd.moduleDescription);
  assert(sampledInfoDescriptions_[InRun].branchType() == InRun);
  preg_.addProductsFromModule(ProductDescriptions{sampledInfoDescriptions_});

  // Specify present products for SubRuns and Runs.  Only the
  // Sampled(Sub)RunInfo products are present for the (Sub)Runs.
  subRunPresentProducts_ = ProductTable{sampledInfoDescriptions_, InSubRun};
  runPresentProducts_ = ProductTable{sampledInfoDescriptions_, InRun};
}

std::unique_ptr<art::FileBlock>
art::SamplingInput::readFile()
{
  for (auto const& name : dataSetSampler_.datasets()) {
    try {
      files_.emplace(
        name,
        detail::SamplingInputFile{name,
                                  dataSetSampler_.fileName(name),
                                  dataSetSampler_.weight(name),
                                  dataSetSampler_.probability(name),
                                  sampledInfoDescriptions_,
                                  readParameterSets_,
                                  preg_});
    }
    catch (Exception const& e) {
      if (e.categoryCode() == errors::FatalRootError) {
        throw Exception{errors::FatalRootError,
                        "A ROOT error occurred in the SamplingInput source.\n"}
          << "For the '" << name
          << "' dataset, the following error occurred:\n\n"
          << e.what() << '\n';
      }
      throw;
    }
  }
  return std::make_unique<art::FileBlock>(
    art::FileFormatVersion{1, "SamplingInput_2018"}, "SamplingInput");
}

void
art::SamplingInput::closeFile()
{
  files_.clear();
}

art::input::ItemType
art::SamplingInput::nextItemType()
{
  if (!inputFileSet_) {
    inputFileSet_ = true;
    return input::IsFile;
  } else if (!runSet_) {
    runSet_ = true;
    return input::IsRun;
  } else if (!subRunSet_) {
    subRunSet_ = true;
    return input::IsSubRun;
  }

  ++generatedEvents_;
  if (generatedEvents_ > maxEvents_) {
    return input::IsStop;
  }

  currentDataset_ = &dataSetSampler_.sample();
  bool const anotherEvent = files_.at(*currentDataset_)
                              .entryForNextEvent(currentEntryInCurrentDataset_);

  // If the sampled data set has no more events, stop.
  if (!anotherEvent) {
    return input::IsStop;
  }

  return input::IsEvent;
}

std::unique_ptr<art::RunPrincipal>
art::SamplingInput::readRun()
{
  art::RunAuxiliary const aux{runID_, nullTimestamp(), nullTimestamp()};
  auto rp = std::make_unique<art::RunPrincipal>(
    aux, pc_, cet::make_exempt_ptr(&runPresentProducts_));
  auto sampledRunInfo = std::make_unique<SampledRunInfo>();
  for (auto const& pr : files_) {
    auto const& dataset = pr.first;
    auto const& file = pr.second;
    sampledRunInfo->emplace(dataset, file.fillRun(*rp));
  }

  // Place sampled run info onto the run
  auto wp = std::make_unique<Wrapper<SampledRunInfo>>(move(sampledRunInfo));
  auto const& pd = sampledInfoDescriptions_[InRun];
  rp->put(std::move(wp),
          pd,
          std::make_unique<ProductProvenance const>(pd.productID(),
                                                    productstatus::present()),
          RangeSet::forRun(runID_));
  return rp;
}

std::unique_ptr<art::SubRunPrincipal>
art::SamplingInput::readSubRun(cet::exempt_ptr<art::RunPrincipal const> rp)
{
  art::SubRunAuxiliary const aux{subRunID_, nullTimestamp(), nullTimestamp()};
  auto srp = std::make_unique<SubRunPrincipal>(
    aux, pc_, cet::make_exempt_ptr(&subRunPresentProducts_));
  auto sampledSubRunInfo = std::make_unique<SampledSubRunInfo>();
  for (auto const& pr : files_) {
    auto const& dataset = pr.first;
    auto const& file = pr.second;
    sampledSubRunInfo->emplace(dataset, file.fillSubRun(*srp));
  }
  srp->setRunPrincipal(rp);

  // Place sampled subrun info onto the subrun
  auto wp =
    std::make_unique<Wrapper<SampledSubRunInfo>>(move(sampledSubRunInfo));
  auto const& pd = sampledInfoDescriptions_[InSubRun];
  srp->put(std::move(wp),
           pd,
           std::make_unique<ProductProvenance const>(pd.productID(),
                                                     productstatus::present()),
           RangeSet::forSubRun(subRunID_));
  return srp;
}

std::unique_ptr<art::EventPrincipal>
art::SamplingInput::readEvent(cet::exempt_ptr<art::SubRunPrincipal const> srp)
{
  ++totalCounts_;
  ++counts_[*currentDataset_];
  detail::issue_reports(totalCounts_, eventID_);

  auto& file = files_.at(*currentDataset_);
  auto ep = file.readEvent(currentEntryInCurrentDataset_, eventID_, pc_);
  ep->setSubRunPrincipal(srp);
  if (!delayedReadEventProducts_) {
    ep->readImmediate();
  }
  eventID_ = eventID_.next();
  return ep;
}

std::unique_ptr<art::RangeSetHandler>
art::SamplingInput::runRangeSetHandler()
{
  return std::make_unique<art::OpenRangeSetHandler>(runID_.run());
}

std::unique_ptr<art::RangeSetHandler>
art::SamplingInput::subRunRangeSetHandler()
{
  return std::make_unique<art::OpenRangeSetHandler>(subRunID_.run());
}

void
art::SamplingInput::doEndJob()
{
  if (!summary_)
    return;

  std::string const spaces(4, ' ');
  std::string const dataset_field{"Dataset"};
  std::size_t width_of_dataset_field{dataset_field.size()};
  cet::for_all(counts_, [& w = width_of_dataset_field](auto const& pr) {
    w = std::max(w, pr.first.size());
  });

  std::string const counts_field{"Counts"};
  std::size_t width_of_counts_field{counts_field.size()};
  cet::for_all(counts_, [& w = width_of_counts_field](auto const& pr) {
    w = std::max(w, std::to_string(pr.second).size());
  });

  std::string const fraction_field{"fraction"};
  std::size_t const width_of_fraction_field{fraction_field.size()};

  std::string const prob_field{"fraction"};
  std::size_t const width_of_prob_field{prob_field.size()};

  std::string const weight_field{"weight"};
  std::string const specified_field{"Specified"};
  std::size_t const width_of_weight_field{specified_field.size()};
  auto const two_column_width =
    width_of_dataset_field + 4 + width_of_counts_field;

  cet::HorizontalRule const rule{
    two_column_width + 4 + width_of_fraction_field + 4 + width_of_prob_field +
    1 + width_of_weight_field};

  mf::LogInfo log{"SamplingInput"};
  log << "The 'SamplingInput' source sampled the following events:\n\n";
  log << std::string(two_column_width, ' ') << spaces << std::left
      << std::setw(width_of_fraction_field) << "Sample"
      << " | " << std::left << std::setw(width_of_prob_field) << "Expected"
      << "  " << std::left << "Specified\n";
  log << std::setw(width_of_dataset_field) << std::left << dataset_field
      << spaces << std::setw(width_of_counts_field) << std::right
      << counts_field << spaces << std::setw(width_of_fraction_field)
      << fraction_field << " | " << std::setw(width_of_prob_field) << prob_field
      << "  " << std::setw(width_of_weight_field) << weight_field << '\n';
  log << rule('-');

  for (auto const& pr : counts_) {
    auto const& dataset = pr.first;
    auto const k = pr.second;
    auto const f = static_cast<double>(k) / totalCounts_;
    log << '\n'
        << std::setw(width_of_dataset_field) << std::left << dataset << spaces
        << std::setw(width_of_counts_field) << std::right << k << spaces
        << std::setw(width_of_fraction_field) << std::right << f << " | "
        << std::setw(width_of_prob_field) << std::right
        << dataSetSampler_.probability(dataset) << "  "
        << std::setw(width_of_weight_field) << std::right
        << dataSetSampler_.weight(dataset);
  }
  log << '\n' << rule('-');
  log << '\n'
      << std::setw(width_of_dataset_field) << std::left << "Total" << spaces
      << std::setw(width_of_counts_field) << std::right << totalCounts_;
}

DEFINE_ART_INPUT_SOURCE(art::SamplingInput)
