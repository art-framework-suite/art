#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/fwd.h"
#include "fhiclcpp/types/Atom.h"

#include <atomic>
#include <cassert>

namespace {
  class EventIDFilterChecker : public art::EDAnalyzer {
  public:
    struct Config {
      fhicl::Atom<unsigned> expPassedEvents{fhicl::Name{"expPassedEvents"}};
    };

    using Parameters = Table<Config>;
    explicit EventIDFilterChecker(Parameters const& p)
      : art::EDAnalyzer{p}, expPassedEvents_{p().expPassedEvents()}
    {}

  private:
    unsigned const expPassedEvents_;
    std::atomic<unsigned> nPassedEvents_{};

    void
    analyze(art::Event const&) override
    {
      ++nPassedEvents_;
    }

    void
    endJob() override
    {
      assert(nPassedEvents_ == expPassedEvents_);
    }
  };
}

DEFINE_ART_MODULE(EventIDFilterChecker)
