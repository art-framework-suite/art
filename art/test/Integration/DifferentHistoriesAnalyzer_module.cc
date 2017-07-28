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

  using ProductTokens_t = std::vector<ProductToken<Ptr<int>>>;

  template <typename T>
  void expected_value_in_ptr(T const& t, ProductTokens_t const&, int expected_value);

  ProductTokens_t srPtrTokens_;
  ProductTokens_t ePtrTokens_;
};


art::test::DifferentHistoriesAnalyzer::DifferentHistoriesAnalyzer(Parameters const& p)
  : EDAnalyzer{p}
{
  auto const& input_label = p().input_label();
  for (auto const& process_name : p().process_names()) {
    srPtrTokens_.push_back(mayConsume<Ptr<int>, InSubRun>(InputTag{input_label, "", process_name}));
    ePtrTokens_.push_back(mayConsume<Ptr<int>, InEvent>(InputTag{input_label, "", process_name}));
  }
}

template <typename T>
void
art::test::DifferentHistoriesAnalyzer::expected_value_in_ptr(T const& t, ProductTokens_t const& tokens, int const expected_value)
{
  unsigned present{}, not_present{};

  for (auto const& token : tokens) {
    Handle<Ptr<int>> h;
    if (t.getByToken(token, h)) {
      ++present;
      BOOST_REQUIRE_EQUAL(**h, expected_value);
    }
    else {
      ++not_present;
    }
  }

  BOOST_REQUIRE_EQUAL(present, 1u);
  BOOST_REQUIRE_EQUAL(not_present, 1u);
}

void art::test::DifferentHistoriesAnalyzer::beginSubRun(SubRun const& sr) {
  expected_value_in_ptr(sr, srPtrTokens_, 5);
}

void art::test::DifferentHistoriesAnalyzer::analyze(Event const& e) {
  expected_value_in_ptr(e, ePtrTokens_, 4);
}

DEFINE_ART_MODULE(art::test::DifferentHistoriesAnalyzer)
