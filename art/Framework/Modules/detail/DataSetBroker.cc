#include "art/Framework/Modules/detail/DataSetBroker.h"
#include "art/Framework/Core/GroupSelectorRules.h"
#include "art/Framework/Modules/detail/event_start.h"
#include "art/Utilities/bold_fontify.h"
#include "cetlib/HorizontalRule.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/Table.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <iomanip>
#include <map>

using namespace art;
using fhicl::Atom;
using fhicl::Comment;
using fhicl::Name;
using fhicl::Sequence;
using fhicl::Table;

namespace {
  struct DataSetConfig {
    Sequence<std::string> fileNames{Name{"fileNames"}};
    Atom<double> weight{Name{"weight"}};
    Atom<std::string> skipToEvent{
      Name{"skipToEvent"},
      Comment{"The 'skipToEvent' parameter specifies the first event that\n"
              "should be used when sampling from a specific input file.  The\n"
              "correct specification is via the triplet \"run:subrun:event\".\n"
              "For example, to begin sampling at run 3, subrun 2, event 39\n"
              "from this dataset, the user should specify\n\n"
              "  skipToEvent: \"3:2:39\"\n\n"
              "Specifying the empty string (the default) is equivalent to\n"
              "beginning at the first event of the file.  All other\n"
              "specifications are ill-formed and will result in an exception\n"
              "throw at the beginning of the job."},
      ""};
  };

  auto
  make_exception_for(std::string const& dataset)
  {
    return Exception{errors::Configuration}
           << "\nModule label: " << art::detail::bold_fontify("source")
           << "\nmodule_type : " << art::detail::bold_fontify("SamplingInput")
           << "\ndataset     : " << art::detail::bold_fontify(dataset);
  }

  auto
  first_event_id(std::string const& firstEvent)
  {
    if (firstEvent.empty()) {
      // Returning an invalid event as the first event is supported
      // because invalid events always compare less than valid events.
      return EventID::invalidEvent();
    }
    RunNumber_t r;
    SubRunNumber_t sr;
    EventNumber_t e;
    std::tie(r, sr, e) = art::detail::event_start(firstEvent);
    return EventID{r, sr, e};
  }
}

detail::DataSetBroker::DataSetBroker(fhicl::ParameterSet const& pset) noexcept(
  false)
{
  auto const dataset_names = pset.get_pset_names();
  if (dataset_names.empty()) {
    throw Exception{
      errors::Configuration,
      "An error occurred while processing dataset configurations.\n"}
      << "No datasets were configured for the SamplingInput source.\n"
         "At least one must be specified.\n";
  }
  auto const ndatasets = dataset_names.size();
  std::vector<std::string> datasetNames;
  std::vector<double> weights;
  datasetNames.reserve(ndatasets);
  weights.reserve(ndatasets);
  for (auto const& dataset : dataset_names) {
    try {
      Table<DataSetConfig> table{pset.get<fhicl::ParameterSet>(dataset)};
      datasetNames.push_back(dataset);
      counts_[dataset] = 0;
      weights.push_back(table().weight());
      auto const filenames = table().fileNames();
      if (filenames.size() != 1ull) {
        throw make_exception_for(dataset)
          << "\n\n"
             "The 'fileNames' sequence must contain 1 and only 1 filename.\n"
             "The ability to specify multiple file names may be possible in "
             "the future.\n"
             "Please contact artists@fnal.gov for guidance.\n\n";
      }

      auto const firstEvent = table().skipToEvent();
      configs_.emplace(dataset,
                       Config{filenames.front(), first_event_id(firstEvent)});
    }
    catch (fhicl::detail::validationException const& e) {
      throw make_exception_for(dataset) << "\n\n" << e.what();
    }
  }
  dataSetSampler_ = std::make_unique<DataSetSampler>(datasetNames, weights);

  mf::LogInfo log{"SamplingInput"};
  log << "The following datasets have been configured for the SamplingInput "
         "source:\n\n";

  std::string const spaces(4, ' ');
  std::string const dataset_field{"Dataset"};
  std::size_t width_of_dataset_field{dataset_field.size()};
  cet::for_all(configs_, [& w = width_of_dataset_field](auto const& pr) {
    w = std::max(w, pr.first.size());
  });

  std::string const specified_field{"Specified"};
  std::string const weight_field{"weight"};
  std::size_t const width_of_weight_field{specified_field.size()};

  std::string const expected_field{"Expected"};
  std::string const fraction_field{"fraction"};
  std::size_t const width_of_fraction_field{expected_field.size()};

  std::string const filename_field{"File name"};
  std::size_t width_of_filename_field{filename_field.size()};
  cet::for_all(configs_, [& w = width_of_filename_field](auto const& pr) {
    w = std::max(w, pr.second.fileName.size());
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

  for (auto const& pr : configs_) {
    auto const& name = pr.first;
    auto const& config = pr.second;
    log << '\n'
        << std::setw(width_of_dataset_field) << std::left << name << spaces
        << std::setw(width_of_weight_field) << std::right
        << dataSetSampler_->weight(name) << spaces
        << std::setw(width_of_fraction_field) << std::right
        << dataSetSampler_->probability(name) << spaces << std::left
        << config.fileName;
    if (config.firstEvent.isValid()) {
      log << "\n  └─ starting at " << config.firstEvent;
    }
  }
}

std::map<BranchKey, BranchDescription>
detail::DataSetBroker::openInputFiles(
  std::vector<std::string> const& inputCommands,
  bool const dropDescendants,
  unsigned int const treeCacheSize,
  int64_t const treeMaxVirtualSize,
  int64_t const saveMemoryObjectThreshold,
  BranchDescription const& sampledEventInfoDesc,
  ModuleDescription const& md,
  bool const readParameterSets,
  MasterProductRegistry& preg)
{
  GroupSelectorRules const groupSelectorRules{
    inputCommands, "inputCommands", "InputSource"};
  std::map<BranchKey, BranchDescription> oldKeyToSampledDescription;
  for (auto const& pr : configs_) {
    auto const& name = pr.first;
    auto const& config = pr.second;
    try {
      files_.emplace(
        name,
        detail::SamplingInputFile{name,
                                  config.fileName,
                                  dataSetSampler_->weight(name),
                                  dataSetSampler_->probability(name),
                                  config.firstEvent,
                                  groupSelectorRules,
                                  dropDescendants,
                                  treeCacheSize,
                                  treeMaxVirtualSize,
                                  saveMemoryObjectThreshold,
                                  sampledEventInfoDesc,
                                  oldKeyToSampledDescription,
                                  md,
                                  readParameterSets,
                                  preg});
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
  return oldKeyToSampledDescription;
}

bool
detail::DataSetBroker::canReadEvent()
{
  currentDataset_ = &dataSetSampler_->sample();
  bool const canRead = files_.at(*currentDataset_).readyForNextEvent();
  if (canRead) {
    ++counts_[*currentDataset_];
    ++totalCounts_;
  }
  return canRead;
}

std::unique_ptr<SampledRunInfo>
detail::DataSetBroker::readAllRunProducts(Products_t& read_products)
{
  auto sampledRunInfo = std::make_unique<SampledRunInfo>();
  for (auto& pr1 : files_) {
    auto const& dataset = pr1.first;
    auto& file = pr1.second;
    auto const entries = file.treeEntries(InRun);

    auto products = file.productsFor(entries, InRun);
    for (auto& pr2 : products) {
      auto const& old_key = pr2.first;
      read_products[old_key][dataset] = std::move(pr2.second);
    }

    sampledRunInfo->emplace(dataset, file.sampledInfoFor<RunID>(entries));
  }
  return sampledRunInfo;
}

std::unique_ptr<SampledSubRunInfo>
detail::DataSetBroker::readAllSubRunProducts(Products_t& read_products)
{
  auto sampledSubRunInfo = std::make_unique<SampledSubRunInfo>();
  for (auto& pr1 : files_) {
    auto const& dataset = pr1.first;
    auto& file = pr1.second;
    auto const entries = file.treeEntries(InSubRun);

    auto products = file.productsFor(entries, InSubRun);
    for (auto& pr2 : products) {
      auto const& old_key = pr2.first;
      read_products[old_key][dataset] = std::move(pr2.second);
    }

    sampledSubRunInfo->emplace(dataset, file.sampledInfoFor<SubRunID>(entries));
  }
  return sampledSubRunInfo;
}

std::unique_ptr<EventPrincipal>
detail::DataSetBroker::readNextEvent(EventID const& id,
                                     ProcessConfigurations const& sampled_pcs,
                                     ProcessConfiguration const& current_pc)
{
  return files_.at(*currentDataset_).readEvent(id, sampled_pcs, current_pc);
}

void
detail::DataSetBroker::countSummary() const
{
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

  auto const two_column_width =
    width_of_dataset_field + 4 + width_of_counts_field;

  std::string const fraction_field{"fraction"};
  std::size_t const width_of_fraction_field{fraction_field.size()};

  std::string const prob_field{"fraction"};
  std::size_t const width_of_prob_field{prob_field.size()};

  std::string const specified_field{"Specified"};
  std::string const weight_field{"weight"};
  std::size_t const width_of_weight_field{specified_field.size()};

  std::string const next_event_field{"Next event"};
  std::size_t const width_of_next_event_field{35u};
  cet::HorizontalRule const rule{
    two_column_width + 4 + width_of_fraction_field + 4 + width_of_prob_field +
    1 + width_of_weight_field + 3 + width_of_next_event_field};

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
      << "  " << std::setw(width_of_weight_field) << weight_field << "   "
      << next_event_field << '\n';
  log << rule('-');

  for (auto const& pr : counts_) {
    auto const& dataset = pr.first;
    auto const k = pr.second;
    auto const f = static_cast<double>(k) / totalCounts_;
    auto const id = files_.at(dataset).nextEvent();
    log << '\n'
        << std::setw(width_of_dataset_field) << std::left << dataset << spaces
        << std::setw(width_of_counts_field) << std::right << k << spaces
        << std::setw(width_of_fraction_field) << std::right << f << " | "
        << std::setw(width_of_prob_field) << std::right
        << dataSetSampler_->probability(dataset) << "  "
        << std::setw(width_of_weight_field) << std::right
        << dataSetSampler_->weight(dataset) << "   ";
    if (id.isValid()) {
      log << id;
    } else {
      log << "(no more available)";
    }
  }
  log << '\n' << rule('-');
  log << '\n'
      << std::setw(width_of_dataset_field) << std::left << "Total" << spaces
      << std::setw(width_of_counts_field) << std::right << totalCounts_;
}
