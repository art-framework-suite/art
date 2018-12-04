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
#include "art/Framework/Modules/SampledInfo.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/DelegatedParameter.h"
#include "fhiclcpp/types/Table.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cassert>
#include <string>

namespace art {
  namespace test {
    class SamplingAnalyzer;
  }
}

using namespace fhicl;

namespace {
  struct DataSetConfig {
    Atom<unsigned int> sampled_run{Name{"sampled_run"}};
    Atom<unsigned int> sampled_subrun{Name{"sampled_subrun"}};
    Atom<int> ivalue{Name{"ivalue"}};
  };

  struct DataSetInfo {
    unsigned int sampled_run;
    unsigned int sampled_subrun;
    int ivalue;
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
      result.emplace(name,
                     DataSetInfo{table().sampled_run(),
                                 table().sampled_subrun(),
                                 table().ivalue()});
    }
    return result;
  }

  template <typename T>
  struct ValidatedIDs {
    explicit ValidatedIDs(std::set<T> expectedIDs)
      : missingIDs{move(expectedIDs)}
    {}
    void
    markID(T const& r)
    {
      auto const numErased = missingIDs.erase(r);
      if (numErased == 0) {
        extraIDs.insert(r);
      }
    }

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
}

class art::test::SamplingAnalyzer : public EDAnalyzer {
public:
  struct Config {
    Atom<std::string> int_label{Name{"int_label"}};
    Atom<std::string> ptrmv_label{Name{"ptrmv_label"}};
    DelegatedParameter datasets{
      Name{"datasets"},
      Comment{
        "The 'datasets' parameter is a table containing a configuration\n"
        "table for each dataset to be read.  It should have the form:\n\n"
        "datasets: {\n"
        "  <dataset>: {\n"
        "    sampled_run: <unsigned int>\n"
        "    sampled_subrun: <unsigned int>\n"
        "    ivalue: <int> \n"
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
  InputTag const intTag_;
  InputTag const ptrmvTag_;
  datasets_t const expectedValues_;
};

art::test::SamplingAnalyzer::SamplingAnalyzer(Parameters const& p)
  : EDAnalyzer{p}
  , intTag_{p().int_label()}
  , ptrmvTag_{p().ptrmv_label()}
  , expectedValues_{convert(p().datasets.get<ParameterSet>())}
{}

void
art::test::SamplingAnalyzer::beginRun(Run const& r)
{
  auto const& hSampledInfo = r.getValidHandle<SampledRunInfo>("SamplingInput");
  auto const& sampledInfoProv = *hSampledInfo.provenance();
  assert(sampledInfoProv.isValid());
  assert(sampledInfoProv.isPresent());

  // Verify that datasets present in the product are expected.
  for (auto const& pr : *hSampledInfo) {
    auto const& dataset = pr.first;
    auto it = expectedValues_.find(dataset);
    assert(it != cend(expectedValues_));

    std::set<RunID> expectedRunIDs{RunID{it->second.sampled_run}};
    ValidatedIDs<RunID> validated{move(expectedRunIDs)};
    auto const& runInfo = pr.second;
    // Verify correct run numbers for each dataset
    cet::for_all(runInfo.ids,
                 [&validated](auto const& id) { validated.markID(id); });
    validated.assert_no_errors();
  }
}

void
art::test::SamplingAnalyzer::beginSubRun(SubRun const& sr)
{
  auto const& hSampledInfo =
    sr.getValidHandle<SampledSubRunInfo>("SamplingInput");
  auto const& sampledInfoProv = *hSampledInfo.provenance();
  assert(sampledInfoProv.isValid());
  assert(sampledInfoProv.isPresent());

  // Verify that datasets present in the product are expected.
  for (auto const& pr : *hSampledInfo) {
    auto const& dataset = pr.first;
    auto it = expectedValues_.find(dataset);
    assert(it != cend(expectedValues_));

    auto const& expected = it->second;
    std::set<SubRunID> expectedSubRunIDs{
      SubRunID{expected.sampled_run, expected.sampled_subrun}};
    ValidatedIDs<SubRunID> validated{move(expectedSubRunIDs)};
    auto const& subRunInfo = pr.second;
    // Verify correct run numbers for each dataset
    cet::for_all(subRunInfo.ids,
                 [&validated](auto const& id) { validated.markID(id); });
    validated.assert_no_errors();
  }
}

void
art::test::SamplingAnalyzer::analyze(Event const& e)
{
  // Test that provenance is available for injected product
  auto const& hSampledInfo =
    e.getValidHandle<SampledEventInfo>("SamplingInput");
  auto const& sampledInfoProv = *hSampledInfo.provenance();
  assert(sampledInfoProv.isValid());
  assert(sampledInfoProv.isPresent());

  auto const& sampledInfo = *hSampledInfo;
  auto const& expected = expectedValues_.at(sampledInfo.dataset);

  // Test injected SampledEventInfo from input source
  assert(expected.sampled_run == sampledInfo.id.run());

  // Verify that persisted event products are stored correctly
  auto const& hInt = e.getValidHandle<arttest::IntProduct>(intTag_);
  assert(expected.ivalue == hInt->value);

  // Check that provenance is available
  auto const& provenance = *hInt.provenance();
  assert(provenance.isValid());
  assert(provenance.isPresent());

  // Verify that persisted art::Ptrs are stored correctly
  auto const& hPtr = e.getValidHandle<art::Ptr<std::string>>(ptrmvTag_);
  assert(**hPtr == "TWO");
}

DEFINE_ART_MODULE(art::test::SamplingAnalyzer)
