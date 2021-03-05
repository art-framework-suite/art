#include "boost/test/unit_test.hpp"

#include "art/Framework/Core/SharedAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/fwd.h"
#include "fhiclcpp/types/Atom.h"

namespace {
  class EventCounter : public art::SharedAnalyzer {
  public:
    struct Config {
      fhicl::Atom<unsigned> expected{
        fhicl::Name{"expected"},
        fhicl::Comment{
          "The 'expected' parameter specifies the number of events that are\n"
          "expected to be processed, based on the corresponding "
          "'SelectEvents'\n"
          "and 'RejectEvents' parameters."}};
    };
    using Parameters = Table<Config>;
    explicit EventCounter(Parameters const& p, art::ProcessingFrame const&)
      : SharedAnalyzer{p}
      , expected_{p().expected()}
    {
      async<art::InEvent>();
    }

  private:
    void
    analyze(art::Event const&, art::ProcessingFrame const&) override
    {
      ++n_;
    }

    void
    endJob(art::ProcessingFrame const&) override
    {
      BOOST_TEST(n_ == expected_);
    }

    unsigned const expected_;
    std::atomic<unsigned> n_{};
  };
}

DEFINE_ART_MODULE(EventCounter)
