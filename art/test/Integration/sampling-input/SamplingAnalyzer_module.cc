////////////////////////////////////////////////////////////////////////
// Class:       SamplingAnalyzer
// Plugin Type: analyzer (art v2_11_05)
// File:        SamplingAnalyzer_module.cc
//
// Generated at Fri Nov 30 10:44:30 2018 by Kyle Knoepfel using cetskelgen
// from cetlib version v3_03_01.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Persistency/Common/Sampled.h"
#include "canvas/Persistency/Provenance/SampledInfo.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/DelegatedParameter.h"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/Table.h"
#include "fhiclcpp/types/TupleAs.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cassert>
#include <string>

namespace art {
  namespace test {
    class SamplingAnalyzer;
  }
}

using namespace art;
using namespace fhicl;

namespace {
  struct DataSetConfig {
    Atom<std::string> process_name{Name{"process_name"}};
    Sequence<TupleAs<SubRunID(unsigned int, unsigned int)>> sampled_subruns{
      Name{"sampled_subruns"}};
    Sequence<Tuple<unsigned, int>> run_values{Name{"run_values"}};
    Sequence<Tuple<Tuple<unsigned, unsigned>, int>> subrun_values{
      Name{"subrun_values"}};
    Sequence<int> event_values{Name{"event_values"}};
  };

  struct DataSetInfo {
    std::vector<SubRunID> sampled_subruns;
    std::map<RunID, int> run_values;
    std::map<SubRunID, int> subrun_values;
    std::vector<int> event_values;
  };

  using datasets_t = std::map<std::string, DataSetInfo>;

  auto
  convert(ParameterSet const& datasets)
  {
    datasets_t result;
    auto const names = datasets.get_pset_names();
    for (auto const& name : names) {
      auto const pset = datasets.get<ParameterSet>(name);
      Table<DataSetConfig> table{pset};
      auto run_value_tuples = table().run_values();
      auto subrun_value_tuples = table().subrun_values();
      std::map<RunID, int> run_values;
      for (auto const tup : run_value_tuples) {
        run_values.emplace(std::get<0>(tup), std::get<1>(tup));
      }
      std::map<SubRunID, int> subrun_values;
      for (auto const tup : subrun_value_tuples) {
        auto const& subrun_tup = std::get<0>(tup);
        subrun_values.emplace(
          SubRunID{std::get<0>(subrun_tup), std::get<1>(subrun_tup)},
          std::get<1>(tup));
      }

      result.emplace(name,
                     DataSetInfo{table().sampled_subruns(),
                                 move(run_values),
                                 move(subrun_values),
                                 table().event_values()});
    }
    return result;
  }

  template <typename T>
  auto to_ids(DataSetInfo const& info);

  template <>
  auto
  to_ids<RunID>(DataSetInfo const& info)
  {
    std::set<RunID> result;
    for (auto const& id : info.sampled_subruns) {
      result.insert(id.runID());
    }
    return result;
  }

  template <>
  auto
  to_ids<SubRunID>(DataSetInfo const& info)
  {
    return std::set<SubRunID>{cbegin(info.sampled_subruns),
                              cend(info.sampled_subruns)};
  }

  template <typename T>
  class ValidatedIDs {
  public:
    explicit ValidatedIDs(DataSetInfo const& info) : missingIDs{to_ids<T>(info)}
    {}

    void
    assert_correct_ids(std::vector<T> const& ids)
    {
      for (auto const& id : ids) {
        if (missingIDs.erase(id) == 0) {
          extraIDs.insert(id);
        }
      }
      assert_no_errors();
    }

  private:
    void
    assert_no_errors() const
    {
      if (missingIDs.empty() && extraIDs.empty()) {
        return;
      }

      cet::for_all(missingIDs, [](auto const& id) {
        std::cerr << " Missing id: " << id << '\n';
      });
      cet::for_all(extraIDs, [](auto const& id) {
        std::cerr << " Extra id: " << id << '\n';
      });
      std::abort();
    }

    std::set<T> missingIDs;
    std::set<T> extraIDs;
  };

  template <typename T>
  void
  assert_correct_ids(
    std::map<std::string, SampledInfo<T>> const& infoForDataset,
    datasets_t const& expectedValues)
  {
    for (auto const& pr : infoForDataset) {
      auto const& dataset = pr.first;
      auto it = expectedValues.find(dataset);
      assert(it != cend(expectedValues));

      auto const& expected_ids = it->second;
      ValidatedIDs<T> validated{expected_ids};
      validated.assert_correct_ids(pr.second.ids);
    }
  }

  template <typename DataContainer>
  auto const&
  expected_values_for(DataSetInfo const& info)
  {
    return info.run_values;
  }

  template <>
  auto const&
  expected_values_for<SubRun>(DataSetInfo const& info)
  {
    return info.subrun_values;
  }

  template <typename DataContainer>
  void
  assert_correct_products(DataContainer const& dc,
                          InputTag const& original_tag,
                          datasets_t const& datasets,
                          unsigned const expected_retrievals)
  {
    InputTag const sampled_tag{original_tag.label(),
                               original_tag.instance(),
                               sampled_from(original_tag.process())};
    auto const& sampledInts =
      *dc.template getValidHandle<Sampled<arttest::IntProduct>>(sampled_tag);
    assert(sampledInts.originalInputTag() == original_tag);

    std::size_t successes{};
    for (auto const& pr : datasets) {
      auto const& dataset = pr.first;
      auto const& dataset_values = pr.second;
      auto const& expected_values =
        expected_values_for<DataContainer>(dataset_values);
      for (auto const& pr2 : expected_values) {
        auto const& id = pr2.first;
        auto const expected_int = pr2.second;
        auto const actual_int = sampledInts.get(dataset, id);
        if (!actual_int) {
          // Could not find product for this dataset/id.  This is not
          // necessarily an error.  Due to us looping through the
          // datasets, there are some datasets that do not have
          // products with a certain signature.  The purpose of the
          // 'successes' counter is to check that we get the correct
          // number of retrieved values for a given product signature.
          // A better design would be to make sure that not only do we
          // get the right number of values, but they correspond to
          // the correct dataset.
          continue;
        }
        assert(actual_int->value == expected_int);
        ++successes;
      }
    }
    assert(successes == expected_retrievals);
  }
}

class art::test::SamplingAnalyzer : public EDAnalyzer {
public:
  struct Config {
    Atom<std::string> run_int_label{Name{"run_int_label"}};
    Atom<std::string> subrun_int_label{Name{"subrun_int_label"}};
    Atom<std::string> event_int_label{Name{"event_int_label"}};
    Atom<std::string> ptrmv_label{Name{"ptrmv_label"}};
    DelegatedParameter datasets{
      Name{"datasets"},
      Comment{
        "The 'datasets' parameter is a table containing a configuration\n"
        "table for each dataset to be read.  It should have the form:\n\n"
        "datasets: {\n"
        "  <dataset>: {\n"
        "    sampled_subruns: [\n"
        "      [<unsigned int>, <unsigned int>],\n"
        "      ...\n"
        "    ]\n"
        "    run_value: <int> \n"
        "    subrun_value: <int> \n"
        "    event_value: <int> \n"
        "  }\n"
        "}\n\n"
        "where '<dataset>' is a string with the correct dataset name,\n"
        "'sampled_run' is the expected run number for the sampled events,\n"
        "in the corresponding dataset, 'sampled_subrun' is the expected\n"
        "subrun number, and 'ivalue' is the expected value for the IntProduct\n"
        "event product."}};
  };

  using Parameters = Table<Config>;
  explicit SamplingAnalyzer(Parameters const& p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  SamplingAnalyzer(SamplingAnalyzer const&) = delete;
  SamplingAnalyzer(SamplingAnalyzer&&) = delete;
  SamplingAnalyzer& operator=(SamplingAnalyzer const&) = delete;
  SamplingAnalyzer& operator=(SamplingAnalyzer&&) = delete;

private:
  void beginRun(art::Run const& r) override;
  void beginSubRun(art::SubRun const& sr) override;
  void analyze(art::Event const& e) override;
  std::string const runIntLabel_;
  std::string const subRunIntLabel_;

  InputTag const eventIntTag_;
  InputTag const ptrmvTag_;
  datasets_t const expectedValues_;
};

art::test::SamplingAnalyzer::SamplingAnalyzer(Parameters const& p)
  : EDAnalyzer{p}
  , runIntLabel_{p().run_int_label()}
  , subRunIntLabel_{p().subrun_int_label()}
  , eventIntTag_{p().event_int_label()}
  , ptrmvTag_{p().ptrmv_label()}
  , expectedValues_{convert(p().datasets.get<ParameterSet>())}
{}

void
art::test::SamplingAnalyzer::beginRun(Run const& r)
{
  auto const hSampledInfo = r.getValidHandle<SampledRunInfo>("SamplingInput");
  auto const& sampledInfoProv = *hSampledInfo.provenance();
  assert(sampledInfoProv.isValid());
  assert(sampledInfoProv.isPresent());
  assert_correct_ids(*hSampledInfo, expectedValues_);

  InputTag const original_signal_and_background_tag{
    runIntLabel_, {}, "SamplingInputWrite"};
  assert_correct_products(
    r, original_signal_and_background_tag, expectedValues_, 2u);

  InputTag const original_noise_tag{runIntLabel_, {}, "RandomNoise"};
  assert_correct_products(r, original_noise_tag, expectedValues_, 1u);
}

void
art::test::SamplingAnalyzer::beginSubRun(SubRun const& sr)
{
  auto const hSampledInfo =
    sr.getValidHandle<SampledSubRunInfo>("SamplingInput");
  auto const& sampledInfoProv = *hSampledInfo.provenance();
  assert(sampledInfoProv.isValid());
  assert(sampledInfoProv.isPresent());
  assert_correct_ids(*hSampledInfo, expectedValues_);

  InputTag const original_signal_and_background_tag{
    subRunIntLabel_, {}, "SamplingInputWrite"};
  assert_correct_products(
    sr, original_signal_and_background_tag, expectedValues_, 3u);

  InputTag const original_noise_tag{subRunIntLabel_, {}, "RandomNoise"};
  assert_correct_products(sr, original_noise_tag, expectedValues_, 1u);
}

void
art::test::SamplingAnalyzer::analyze(Event const& e)
{
  // Test that provenance is available for injected product
  auto const hSampledInfo = e.getValidHandle<SampledEventInfo>("SamplingInput");
  auto const& sampledInfoProv = *hSampledInfo.provenance();
  assert(sampledInfoProv.isValid());
  assert(sampledInfoProv.isPresent());

  auto const& sampledInfo = *hSampledInfo;
  auto const& expected = expectedValues_.at(sampledInfo.dataset);

  // Test injected SampledEventInfo from input source
  assert(cet::search_all(expected.sampled_subruns, sampledInfo.id.subRunID()));

  // Verify that persisted event products are stored correctly
  auto const hInt = e.getValidHandle<arttest::IntProduct>(eventIntTag_);
  assert(expected.event_values[0] == hInt->value);

  // Check that provenance is available
  auto const& provenance = *hInt.provenance();
  assert(provenance.isValid());
  assert(provenance.isPresent());

  // Verify that persisted art::Ptrs are stored correctly
  auto const hPtr = e.getValidHandle<art::Ptr<std::string>>(ptrmvTag_);
  assert(**hPtr == "TWO");
}

DEFINE_ART_MODULE(art::test::SamplingAnalyzer)
