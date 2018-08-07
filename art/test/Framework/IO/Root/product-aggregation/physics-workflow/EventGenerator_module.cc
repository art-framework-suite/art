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

#include <atomic>

namespace {

  auto
  produce_particle_ids(std::size_t const nParticles)
  {
    auto particle_ids = std::make_unique<std::vector<int>>(nParticles);
    auto i = static_cast<int>(-nParticles);
    std::generate(begin(*particle_ids), end(*particle_ids), [&i] {
      auto const value = i;
      i += 2;
      return value;
    });
    return particle_ids;
  }

  class EventGenerator : public art::EDProducer {
    static constexpr unsigned maxParticleSize_{10u};
    std::atomic<unsigned> nPOTs_{};
    std::atomic<unsigned> nParticles_{};

  public:
    struct Config {
    };
    using Parameters = EDProducer::Table<Config>;
    explicit EventGenerator(Parameters const& ps) : EDProducer{ps}
    {
      produces<std::vector<int>>("GenParticles");
      produces<unsigned, art::InSubRun>("nParticles");
      produces<unsigned, art::InRun>("nPOTs");
    }

    void
    produce(art::Event& e) override
    {
      auto const n = (e.id().event() % maxParticleSize_);
      nParticles_ += n;
      e.put(produce_particle_ids(n), "GenParticles");
      ++nPOTs_;
    }

    void
    endSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<unsigned>(nParticles_),
             "nParticles",
             art::subRunFragment());
      nParticles_ = 0u;
    }

    void
    endRun(art::Run& r) override
    {
      r.put(std::make_unique<unsigned>(nPOTs_), "nPOTs", art::runFragment());
      nPOTs_ = 0u;
    }

  }; // EventGenerator

} // namespace

DEFINE_ART_MODULE(EventGenerator)
