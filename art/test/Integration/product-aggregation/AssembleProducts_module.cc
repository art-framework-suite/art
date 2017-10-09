#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/types/TupleAs.h"

using art::InputTag;
using fhicl::Name;
using fhicl::TupleAs;
using std::string;

namespace {

  struct Config {
    TupleAs<InputTag(string)> particlesTag{Name("particlesTag")};
  };

  class AssembleProducts : public art::EDProducer {
  public:
    using Parameters = EDProducer::Table<Config>;
    explicit AssembleProducts(Parameters const& config)
      : particlesTag_{config().particlesTag()}
    {
      produces<unsigned, art::InSubRun>("seenParticles");
    }

    void
    produce(art::Event& e) override
    {
      auto const& particles =
        e.getValidHandle<std::vector<double>>(particlesTag_);
      seenParticles_ += particles->size();
    }

    void
    endSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<unsigned>(seenParticles_),
             "seenParticles",
             art::subRunFragment());
      seenParticles_ = 0u;
    }

  private:
    InputTag particlesTag_;
    unsigned seenParticles_{};

  }; // AssembleProducts
} // namespace

DEFINE_ART_MODULE(AssembleProducts)
