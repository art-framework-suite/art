// ====================================================================
// SamplingInput
//
// - Only one Run and SubRun is created for a job that uses the
//   SamplingInput source.
//
// - Run and SubRun products are available only through the
//   SampledProduct<T> wrapper.  This product wrapper is a container
//   that retains the (Sub)Run products from each dataset.
//
// Technical notes:
//
// - Whereas event principals are created by the individual
//   SamplingInputFile objects, the (sub)run principals are created by
//   the input source itself.  The goal is to encapsulate as much as
//   possible the product internals to the source, and not the
//   underlying files.  This is possible for (sub)run products as
//   there is only one primary (sub)run per job.  For event products,
//   it is more natural to go to the individual input files
//   themselves.  This asymmetry is undesirable, but it might be
//   required due to the conceptual difference between events and
//   (sub)runs.
// ===================================================================

#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceMacros.h"
#include "art/Framework/Core/detail/issue_reports.h"
#include "art/Framework/Modules/detail/DataSetSampler.h"
#include "art/Framework/Modules/detail/SamplingInputFile.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OpenRangeSetHandler.h"
#include "art/Framework/Principal/RangeSetsSupported.h"
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
#include "canvas/Persistency/Provenance/SampledInfo.h"
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
#include <type_traits>
#include <vector>

using namespace fhicl;
using namespace std::string_literals;

namespace {
  using Products_t = std::map<
    art::BranchKey,
    std::map<std::string, std::vector<std::unique_ptr<art::EDProduct>>>>;

  std::unique_ptr<art::EDProduct>
  make_sampled_product(Products_t& read_products,
                       art::BranchKey const& original_key)
  {
    art::InputTag const tag{original_key.moduleLabel_,
                            original_key.productInstanceName_,
                            original_key.processName_};

    auto& datasets_with_product = read_products.at(original_key);

    auto const first_entry = begin(datasets_with_product);
    auto const& products = first_entry->second;
    assert(!products.empty());

    auto sampled_product = products[0]->createEmptySampledProduct(tag);
    for (auto&& pr : datasets_with_product) {
      auto const& dataset = pr.first;
      auto&& products = std::move(pr.second);
      for (auto&& product : products) {
        sampled_product->insertIfSampledProduct(dataset, move(product));
      }
    }
    return sampled_product;
  }
}

namespace art {

  class SamplingInput : public InputSource {
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
          "      fileNames: [<string>]\n"
          "      weight: <double>\n"
          "      skipToEvent: \"\"  #default\n"
          "    }\n"
          "    ...\n"
          "  }\n\n"
          "where the '<dataset name>' parameter labels a particular\n"
          "dataset (e.g. 'signal'), the 'fileNames' refers to the files\n"
          "that contains events of the dataset, and the 'weight'\n"
          "is a floating-point number used to determine the frequency\n"
          "with which events from this dataset are sampled, relative to\n"
          "the sum of weights across all datasets.\n\n"
          "The 'skipToEvent' parameter specifies the first event that\n"
          "should be used when sampling from a specific input file.  The\n"
          "correct specification is via the triplet \"run:subrun:event\".\n"
          "For example, to begin sampling at run 3, subrun 2, event 39\n"
          "from this dataset, the user should specify:\n\n"
          "  skipToEvent: \"3:2:39\"\n\n"
          "Specifying the empty string (the default) is equivalent to\n"
          "beginning at the first event of the file.  All other\n"
          "specifications are ill-formed and will result in an exception\n"
          "throw at the beginning of the job.\n\n"
          "The ellipsis indicates that multiple datasets can be configured.\n\n"
          "N.B. Only one file per dataset is currently allowed."}};
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
                           InputSourceDescription& isd);

  private:
    template <typename T>
    std::enable_if_t<detail::RangeSetsSupported<T::branch_type>::value>
    putSampledProductsInto_(T& principal, Products_t& read_products) const;

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

    ModuleDescription const md_;
    ProcessConfiguration const& pc_;
    MasterProductRegistry& preg_;
    RunID runID_;
    SubRunID subRunID_;
    EventID eventID_;
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
    std::map<BranchKey, BranchDescription> oldKeyToSampledProductDescription_;
    BranchDescription sampledRunInfoDesc_;
    BranchDescription sampledSubRunInfoDesc_;
    BranchDescription sampledEventInfoDesc_;
    ProductTable presentSubRunProducts_;
    ProductTable presentRunProducts_;
    cet::exempt_ptr<std::string const> currentDataset_{nullptr};
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
  , md_{isd.moduleDescription}
  , pc_{md_.processConfiguration()}
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
  ProductDescriptions presentSampledProducts;
  auto const emulated_module_name = "SamplingInput"s;
  presentSampledProducts.emplace_back(
    InEvent,
    TypeLabel{
      TypeID{typeid(SampledEventInfo)}, {}, false, emulated_module_name},
    isd.moduleDescription);
  sampledEventInfoDesc_ = presentSampledProducts.back();
  presentSampledProducts.emplace_back(
    InSubRun,
    TypeLabel{
      TypeID{typeid(SampledSubRunInfo)}, {}, false, emulated_module_name},
    isd.moduleDescription);
  sampledSubRunInfoDesc_ = presentSampledProducts.back();
  presentSampledProducts.emplace_back(
    InRun,
    TypeLabel{TypeID{typeid(SampledRunInfo)}, {}, false, emulated_module_name},
    isd.moduleDescription);
  sampledRunInfoDesc_ = presentSampledProducts.back();

  // Open the input files
  for (auto const& name : dataSetSampler_.datasets()) {
    try {
      std::map<BranchKey, BranchDescription> oldKeyToSampledDescription;
      files_.emplace(
        name,
        detail::SamplingInputFile{name,
                                  dataSetSampler_.fileName(name),
                                  dataSetSampler_.weight(name),
                                  dataSetSampler_.probability(name),
                                  dataSetSampler_.firstEvent(name),
                                  sampledEventInfoDesc_,
                                  oldKeyToSampledDescription,
                                  md_,
                                  readParameterSets_,
                                  preg_});
      for (auto const& pr : oldKeyToSampledDescription) {
        presentSampledProducts.push_back(pr.second);
      }
      auto const& descs = oldKeyToSampledDescription;
      oldKeyToSampledProductDescription_.insert(cbegin(descs), cend(descs));
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

  // It is possible for the same product to specified in multiple
  // input files.  This is not an error, so we thus find the unique
  // set of descriptions.
  cet::sort_all(presentSampledProducts);
  auto current_end = end(presentSampledProducts);
  auto new_end = std::unique(begin(presentSampledProducts), current_end);
  presentSampledProducts.erase(new_end, current_end);

  // Specify present products for SubRuns and Runs.  Only the
  // Sampled(Sub)RunInfo products are present for the (Sub)Runs.
  presentSubRunProducts_ = ProductTable{presentSampledProducts, InSubRun};
  presentRunProducts_ = ProductTable{presentSampledProducts, InRun};

  preg_.addProductsFromModule(ProductDescriptions{presentSampledProducts});
}

std::unique_ptr<art::FileBlock>
art::SamplingInput::readFile()
{
  return std::make_unique<art::FileBlock>(
    art::FileFormatVersion{1, "SamplingInput_2018"}, "SamplingInput");
}

void
art::SamplingInput::closeFile()
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
        << dataSetSampler_.probability(dataset) << "  "
        << std::setw(width_of_weight_field) << std::right
        << dataSetSampler_.weight(dataset) << "   ";
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
  bool const anotherEvent = files_.at(*currentDataset_).readyForNextEvent();

  // If the sampled data set has no more events, stop.
  if (!anotherEvent) {
    return input::IsStop;
  }

  return input::IsEvent;
}

template <typename T>
std::enable_if_t<art::detail::RangeSetsSupported<T::branch_type>::value>
art::SamplingInput::putSampledProductsInto_(T& principal,
                                            Products_t& read_products) const
{
  for (auto const& pr : oldKeyToSampledProductDescription_) {
    auto const& old_key = pr.first;
    if (old_key.branchType_ != principal.branchType())
      continue;

    auto const& sampled_pd = pr.second;

    principal.put(make_sampled_product(read_products, old_key),
                  sampled_pd,
                  std::make_unique<ProductProvenance const>(
                    sampled_pd.productID(), productstatus::present()),
                  RangeSet::forRun(runID_));
  }
}

std::unique_ptr<art::RunPrincipal>
art::SamplingInput::readRun()
{
  art::RunAuxiliary const aux{runID_, nullTimestamp(), nullTimestamp()};

  // Read all run products into memory
  Products_t read_products;
  auto sampledRunInfo = std::make_unique<SampledRunInfo>();
  for (auto& pr : files_) {
    auto const& dataset = pr.first;
    auto& file = pr.second;
    auto const entries = file.treeEntries(InRun);

    auto products = file.productsFor(entries, InRun);
    for (auto& pr : products) {
      auto const& old_key = pr.first;
      auto&& eps = pr.second;
      assert(!eps.empty());
      auto& datasets_with_product = read_products[old_key];
      datasets_with_product[dataset] = std::move(eps);
    }

    // We use a set because it is okay for multiple entries of the
    // same Run to be present in the FileIndex--these correspond to
    // Run fragments.  However, we do not want these to appear as
    // separate entries in the SampledInfo object.
    std::set<RunID> ids;
    for (auto const& pr : entries) {
      auto const& invalid_event_id = pr.first;
      ids.insert(invalid_event_id.runID());
    }

    SampledInfo<RunID> info{dataSetSampler_.weight(dataset),
                            dataSetSampler_.probability(dataset),
                            std::vector<RunID>(cbegin(ids), cend(ids))};
    sampledRunInfo->emplace(dataset, std::move(info));
  }

  auto rp = std::make_unique<art::RunPrincipal>(
    aux, pc_, cet::make_exempt_ptr(&presentRunProducts_));

  putSampledProductsInto_(*rp, read_products);

  // Place sampled run info onto the run
  auto wp = std::make_unique<Wrapper<SampledRunInfo>>(move(sampledRunInfo));
  rp->put(std::move(wp),
          sampledRunInfoDesc_,
          std::make_unique<ProductProvenance const>(
            sampledRunInfoDesc_.productID(), productstatus::present()),
          RangeSet::forRun(runID_));
  return rp;
}

std::unique_ptr<art::SubRunPrincipal>
art::SamplingInput::readSubRun(cet::exempt_ptr<art::RunPrincipal const> rp)
{
  art::SubRunAuxiliary const aux{subRunID_, nullTimestamp(), nullTimestamp()};
  // Read all run products into memory
  Products_t read_products;
  auto sampledSubRunInfo = std::make_unique<SampledSubRunInfo>();
  for (auto& pr : files_) {
    auto const& dataset = pr.first;
    auto& file = pr.second;
    auto const entries = file.treeEntries(InSubRun);

    auto products = file.productsFor(entries, InSubRun);
    for (auto& pr : products) {
      auto const& old_key = pr.first;
      auto&& eps = pr.second;
      assert(!eps.empty());
      auto& datasets_with_product = read_products[old_key];
      datasets_with_product[dataset] = std::move(eps);
    }

    // We use a set because it is okay for multiple entries of the
    // same Run to be present in the FileIndex--these correspond to
    // Run fragments.  However, we do not want these to appear as
    // separate entries in the SampledInfo object.
    std::set<SubRunID> ids;
    for (auto const& pr : entries) {
      auto const& invalid_event_id = pr.first;
      ids.insert(invalid_event_id.subRunID());
    }

    SampledInfo<SubRunID> info{dataSetSampler_.weight(dataset),
                               dataSetSampler_.probability(dataset),
                               std::vector<SubRunID>(cbegin(ids), cend(ids))};
    sampledSubRunInfo->emplace(dataset, std::move(info));
  }

  auto srp = std::make_unique<SubRunPrincipal>(
    aux, pc_, cet::make_exempt_ptr(&presentSubRunProducts_));
  srp->setRunPrincipal(rp);

  putSampledProductsInto_(*srp, read_products);

  // Place sampled run info onto the run
  auto wp =
    std::make_unique<Wrapper<SampledSubRunInfo>>(move(sampledSubRunInfo));
  srp->put(std::move(wp),
           sampledSubRunInfoDesc_,
           std::make_unique<ProductProvenance const>(
             sampledSubRunInfoDesc_.productID(), productstatus::present()),
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
  auto ep = file.readEvent(eventID_, pc_);
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

DEFINE_ART_INPUT_SOURCE(art::SamplingInput)
