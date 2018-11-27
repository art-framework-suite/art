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
#include "art/Framework/Modules/SampledEventID.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "canvas/Utilities/InputTag.h"
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
    Atom<int> ivalue{Name{"ivalue"}};
  };

  struct DataSetInfo {
    unsigned int sampled_run;
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
                     DataSetInfo{table().sampled_run(), table().ivalue()});
    }
    return result;
  }
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
        "    ivalue: <int> \n"
        "  }\n"
        "}\n\n"
        "where '<dataset>' is a string with the correct dataset name,\n"
        "'sampled_run' is the expected run number for the sampled events\n"
        "in the corresponding dataset, and 'ivalue' is the expected value"
        "for the IntProduct event product."}};
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
art::test::SamplingAnalyzer::analyze(Event const& e)
{
  // Test that provenance is available for injected product
  auto const& hSampledID = e.getValidHandle<SampledEventID>("SamplingInput");
  auto const& sampledIDProv = *hSampledID.provenance();
  assert(sampledIDProv.isValid());
  assert(sampledIDProv.isPresent());

  auto const& sampledID = *hSampledID;
  auto const& expected = expectedValues_.at(sampledID.dataset);

  // Test injected SampledEventID from input source
  assert(expected.sampled_run == sampledID.id.run());

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
