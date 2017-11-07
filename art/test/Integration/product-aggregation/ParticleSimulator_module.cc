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
#include "art/test/Integration/product-aggregation/Geometry.h"
#include "fhiclcpp/types/TupleAs.h"

#include <cmath>

namespace {

  struct Config {
    fhicl::TupleAs<art::InputTag(std::string)> inputTag{
      fhicl::Name("inputTag")};
  };

  auto
  get_energies(std::vector<int> const& particle_ids)
  {
    // The energy is arbitrarily calculated to be:
    //   abs(pdg_id) + particle_ids.size();
    auto energies = std::make_unique<std::vector<double>>();
    auto const energy_offset = static_cast<double>(particle_ids.size());
    cet::transform_all(particle_ids,
                       back_inserter(*energies),
                       [&energy_offset] (auto const pid) { return std::abs(pid) + energy_offset; });
    return energies;
  }

  class ParticleSimulator : public art::EDProducer {
    art::ProductToken<std::vector<int>> genParticlesTkn_;

  public:
    using Parameters = EDProducer::Table<Config>;
    explicit ParticleSimulator(Parameters const& config)
      : genParticlesTkn_{consumes<std::vector<int>>(config().inputTag())}
    {
      produces<std::vector<double>>("particleEnergies");
      produces<arttest::Geometry, art::InRun>("Geometry");
    }

    void
    beginRun(art::Run& r) override
    {
      r.put(std::make_unique<arttest::Geometry>(r.run()),
            "Geometry",
            art::fullRun());
    }

    void
    produce(art::Event& e) override
    {
      auto const& genParticles = e.getValidHandle(genParticlesTkn_);
      e.put(get_energies(*genParticles), "particleEnergies");
    }

  }; // ParticleSimulator
}

DEFINE_ART_MODULE(ParticleSimulator)
