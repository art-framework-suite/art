#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Run.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/quiet_unit_test.hpp"
#include "fhiclcpp/types/Atom.h"
#include "test/TestObjects/ToyProducts.h"

#include <string>

namespace {

  struct Config {
    fhicl::Atom<std::string> eventsTag { fhicl::Name("eventsTag") };
    fhicl::Atom<std::string> eventsCollTag { fhicl::Name("eventsCollTag") };
    fhicl::Atom<std::string> nameTag { fhicl::Name("nameTag") };
  };

  class RunProductAnalyzer : public art::EDAnalyzer {
    art::InputTag eventsTag_;
    art::InputTag eventsCollTag_;
    art::InputTag nameTag_;
  public:

    using Parameters = art::EDAnalyzer::Table<Config>;
    RunProductAnalyzer(Parameters const& p)
      : EDAnalyzer{p}
      , eventsTag_{p().eventsTag()}
      , eventsCollTag_{p().eventsCollTag()}
      , nameTag_{p().nameTag()}
    {}

    void beginRun(art::Run const& r) override
    {
      auto totalEvents  = r.getValidHandle<unsigned>(eventsTag_);
      auto combinedProd = r.getValidHandle<std::vector<int> >(eventsCollTag_);
      BOOST_REQUIRE_EQUAL(*totalEvents, combinedProd->size());

      // Exception thrown only if an aggregation for 'std::string' is attempted
      try {
        (void)r.getValidHandle<std::string>(nameTag_);
      }
      catch(art::Exception const& e) {
        BOOST_REQUIRE_EQUAL(e.categoryCode(), art::errors::ProductCannotBeAggregated);
      }
    }

    void analyze(art::Event const&) override {}

  };

}

DEFINE_ART_MODULE(RunProductAnalyzer)
