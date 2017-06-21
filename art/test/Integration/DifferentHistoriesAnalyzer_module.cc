////////////////////////////////////////////////////////////////////////
// Class:       DifferentHistoriesAnalyzer
// Module Type: analyzer
// File:        DifferentHistoriesAnalyzer_module.cc
//
// This analyzer specifically tests that for products that originated
// from two different processes with different process names, the
// presence bits are set appropriately (present/not-present) for
// products that were/were not produced in a given process.
////////////////////////////////////////////////////////////////////////

#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/types/Atom.h"

namespace art {
  namespace test {
    class DifferentHistoriesAnalyzer;
  }
}

class art::test::DifferentHistoriesAnalyzer : public EDAnalyzer {
  struct Config {
    fhicl::Atom<std::string> input_label{fhicl::Name{"input_label"}};
    fhicl::Sequence<std::string> process_names {fhicl::Name{"process_names"}};
  };
public:
  using Parameters = Table<Config>;
  explicit DifferentHistoriesAnalyzer(Parameters const& p);
private:
  void beginSubRun(SubRun const& sr) override;
  void analyze(Event const& e) override;

  template <typename T>
  void expected_value_in_ptr(T const& t, int value);

  std::string input_label_;
  std::vector<std::string> process_names_;
};


art::test::DifferentHistoriesAnalyzer::DifferentHistoriesAnalyzer(Parameters const& p)
  : EDAnalyzer{p},
    input_label_{p().input_label()},
    process_names_{p().process_names()}
{}

template <typename T>
void
art::test::DifferentHistoriesAnalyzer::expected_value_in_ptr(T const& t, int const value)
{
  unsigned present{}, not_present{};

  for (auto const& process_name : process_names_) {
    Handle<Ptr<int>> h;
    if (t.getByLabel(input_label_, "", process_name, h)) {
      ++present;
      BOOST_REQUIRE_EQUAL(**h, value);
    }
    else {
      ++not_present;
    }
  }

  BOOST_REQUIRE_EQUAL(present, 1u);
  BOOST_REQUIRE_EQUAL(not_present, 1u);
}

void art::test::DifferentHistoriesAnalyzer::beginSubRun(SubRun const& sr) {
  expected_value_in_ptr(sr, 5);
}

void art::test::DifferentHistoriesAnalyzer::analyze(Event const& e) {
  expected_value_in_ptr(e, 4);
}

DEFINE_ART_MODULE(art::test::DifferentHistoriesAnalyzer)
