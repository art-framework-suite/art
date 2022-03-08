#include "boost/test/unit_test.hpp"

#include "art/Framework/Core/SharedAnalyzer.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/Globals.h"
#include "fhiclcpp/types/Atom.h"

#include <chrono>

namespace {
  using namespace std::chrono;
  auto now = steady_clock::now;

  class Busy : public art::SharedAnalyzer {
  public:
    struct Config {
      fhicl::Atom<unsigned> numEvents{
        fhicl::Name{"numEvents"},
        fhicl::Comment{"Number of events to be processed by module."}};
      fhicl::Atom<double> waitFor{
        fhicl::Name{"waitFor"},
        fhicl::Comment{"Duration (in seconds) for which the analyze function "
                       "should execute."}};
    };
    using Parameters = Table<Config>;
    explicit Busy(Parameters const& p, art::ProcessingFrame const&)
      : SharedAnalyzer{p}, nEvents_{p().numEvents()}, waitFor_{p().waitFor()}
    {
      async<art::InEvent>();
    }

  private:
    void
    beginJob(art::ProcessingFrame const&) override
    {
      begin_ = now();
    }

    void
    analyze(art::Event const&, art::ProcessingFrame const&) override
    {
      auto const begin = now();
      while (duration_cast<seconds>(now() - begin).count() < waitFor_) {
      }
    }

    void
    endJob(art::ProcessingFrame const&) override
    {
      auto const time_taken = duration_cast<seconds>(now() - begin_).count();
      auto const n_schedules = art::Globals::instance()->nschedules();
      auto const expected_lower_limit = (waitFor_ * nEvents_) / n_schedules;
      BOOST_TEST(time_taken >= expected_lower_limit);
    }

    unsigned int const nEvents_;
    double const waitFor_;
    using time_point_t = std::chrono::steady_clock::time_point;
    time_point_t begin_;
  };
}

DEFINE_ART_MODULE(Busy)
