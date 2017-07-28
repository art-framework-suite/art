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
#include "fhiclcpp/types/TupleAs.h"
#include "art/test/Integration/product-aggregation/Geometry.h"

#include <random>

namespace {

  struct Config {
    fhicl::TupleAs<art::InputTag(std::string)> inputTag { fhicl::Name("inputTag") };
    struct Energies {
      fhicl::Atom<double> lower { fhicl::Name("lower") };
      fhicl::Atom<double> upper { fhicl::Name("upper") };
    };
    fhicl::Table<Energies> energies { fhicl::Name("energyRange") };
  };

  using engine_t = std::default_random_engine;
  using energyDist_t = std::uniform_real_distribution<double>;

  auto get_energies(std::vector<int> const& particles,
                    engine_t& gen,
                    energyDist_t& energyDist)
  {
    std::vector<double> energies (particles.size());
    std::generate(energies.begin(),
                  energies.end(),
                  [&gen,&energyDist]{ return energyDist(gen); } );
    return std::make_unique<decltype(energies)>(energies);
  }

  class ParticleSimulator : public art::EDProducer {
    engine_t gen_ {};
    energyDist_t energyDist_;
    art::ProductToken<std::vector<int>> genParticlesTkn_;
  public:

    using Parameters = EDProducer::Table<Config>;
    explicit ParticleSimulator(Parameters const& config)
      : energyDist_{config().energies().lower(), config().energies().upper()}
      , genParticlesTkn_{consumes<std::vector<int>>(config().inputTag())}
    {
      produces<std::vector<double>>("particleEnergies");
      produces<arttest::Geometry,art::InRun>("Geometry");

    }

    void beginRun(art::Run& r) override
    {
      r.put(std::make_unique<arttest::Geometry>(r.run()), "Geometry", art::fullRun());
    }

    void produce(art::Event& e) override
    {
      auto const& genParticles = e.getValidHandle(genParticlesTkn_);
      e.put(get_energies(*genParticles, gen_, energyDist_), "particleEnergies");
    }


  };  // ParticleSimulator

}

DEFINE_ART_MODULE(ParticleSimulator)
