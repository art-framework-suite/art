#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Run.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/quiet_unit_test.hpp"
#include "fhiclcpp/types/Sequence.h"
#include "fhiclcpp/types/TupleAs.h"
#include "test/TestObjects/ToyProducts.h"

#include <string>
#include <vector>

namespace {

  struct ExpectedRunProds {
    ExpectedRunProds(std::string const& t,
                     unsigned const evtCounts)
      : tag{t}
      , counts{evtCounts}
    {}
    art::InputTag tag;
    unsigned counts;
  };

  struct Config {
    fhicl::Sequence< fhicl::TupleAs< ExpectedRunProds(std::string, unsigned) > >
    products { fhicl::Name("products") };
  };

  class RunProductAnalyzerTiered : public art::EDAnalyzer {
    std::vector<ExpectedRunProds> runProds_;
  public:

    using Parameters = art::EDAnalyzer::Table<Config>;
    RunProductAnalyzerTiered(Parameters const& p)
      : EDAnalyzer{p}
      , runProds_{p().products()}
    {}

    void beginRun(art::Run const& r) override
    {
      for (auto const& eprod : runProds_) {
        auto h = r.getValidHandle<unsigned>(eprod.tag);
        BOOST_CHECK_EQUAL(*h, eprod.counts);
      }
      // auto totalEvents  = r.getValidHandle<unsigned>(eventsTag_);
      // auto combinedProd = r.getValidHandle<std::vector<int> >(eventsCollTag_);
    }

    void analyze(art::Event const&) override {}

  };

}

DEFINE_ART_MODULE(RunProductAnalyzerTiered)
