//--------------------------------------------------------------------
//
// Main motivation for RunProductProducer is to test product
// aggregation.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "fhiclcpp/types/Atom.h"

#include <random>

namespace {

  struct Config {
    fhicl::Atom<int> lower { fhicl::Name("lower") };
    fhicl::Atom<int> upper { fhicl::Name("upper") };
  };

  using engine_t = std::default_random_engine;
  using sizeDist_t = std::uniform_int_distribution<unsigned>;
  using pdgDist_t = std::uniform_int_distribution<int>;

  auto produce_particles(engine_t& gen,
                         sizeDist_t& sizeDist,
                         pdgDist_t& pdgDist)
  {
    std::vector<int> particles (sizeDist(gen));
    std::generate(particles.begin(),
                  particles.end(),
                  [&gen,&pdgDist]{ return pdgDist(gen); } );
    return std::make_unique<decltype(particles)>(particles);
  }

  class EventGenerator : public art::EDProducer {
    engine_t gen_ {};
    sizeDist_t sizeDist_ {0, 10};
    pdgDist_t pdgDist_;
    unsigned nPOTs_ {};
    unsigned nParticles_ {};
  public:

    using Parameters = EDProducer::Table<Config>;
    explicit EventGenerator(Parameters const& config)
      : pdgDist_{config().lower(), config().upper()}
    {
      produces<std::vector<int>>("GenParticles");
      produces<unsigned,art::InSubRun>("nParticles");
      produces<unsigned,art::InRun>("nPOTs");
    }

    void produce(art::Event& e) override
    {
      auto particles = produce_particles(gen_, sizeDist_, pdgDist_);
      nParticles_ += particles->size();
      e.put(std::move(particles), "GenParticles");
      ++nPOTs_;
    }

    void endSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<unsigned>(nParticles_), "nParticles", sr.seenRangeSet());
      nParticles_ = 0u;
    }

    void endRun(art::Run& r) override
    {
      r.put(std::make_unique<unsigned>(nPOTs_), "nPOTs", r.seenRangeSet());
      nPOTs_ = 0u;
    }

  };  // EventGenerator

}

DEFINE_ART_MODULE(EventGenerator)
