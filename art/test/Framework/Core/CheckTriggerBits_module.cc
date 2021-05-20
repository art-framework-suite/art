#include "boost/test/unit_test.hpp"

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/SharedAnalyzer.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/System/TriggerNamesService.h"

namespace {
  class CheckTriggerBits : public art::SharedAnalyzer {
  public:
    struct Config {
      fhicl::Sequence<std::string> ordered_paths{fhicl::Name{"ordered_paths"}};
      fhicl::Atom<bool> expected_a{fhicl::Name{"expected_a"}};
      fhicl::Atom<bool> expected_b{fhicl::Name{"expected_b"}};
    };
    using Parameters = Table<Config>;
    explicit CheckTriggerBits(Parameters const& p, art::ProcessingFrame const&);

  private:
    void analyze(art::Event const&, art::ProcessingFrame const&) override;
    std::vector<std::string> const orderedPaths_;
    bool const expectedA_;
    bool const expectedB_;
    art::ProductToken<art::TriggerResults> const token_;
    art::ServiceHandle<art::TriggerNamesService const> triggerNames_;
  };

  CheckTriggerBits::CheckTriggerBits(Parameters const& p,
                                     art::ProcessingFrame const&)
    : SharedAnalyzer{p}
    , orderedPaths_{p().ordered_paths()}
    , expectedA_{p().expected_a()}
    , expectedB_{p().expected_b()}
    , token_{consumes<art::TriggerResults>("TriggerResults")}
  {
    async<art::InEvent>();

    auto const& trigger_paths = triggerNames_->getTrigPaths();
    auto const num_paths = size(trigger_paths);
    BOOST_TEST(num_paths == size(orderedPaths_));

    for (std::size_t i{}; i != num_paths; ++i) {
      BOOST_TEST(trigger_paths[i] == orderedPaths_[i]);
    }
  }

  void
  CheckTriggerBits::analyze(art::Event const& e, art::ProcessingFrame const&)
  {
    auto const results = triggerNames_->pathResults(e);
    BOOST_TEST(size(results) == 2ull);
    BOOST_TEST(results.at("a").accept() == expectedA_);
    BOOST_TEST(results.at("b").accept() == expectedB_);
  }
}

DEFINE_ART_MODULE(CheckTriggerBits)
