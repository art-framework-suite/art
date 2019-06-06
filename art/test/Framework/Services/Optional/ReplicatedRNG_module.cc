#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/ReplicatedProducer.h"

#include <cassert>

namespace {
  class ReplicatedRNG : public art::ReplicatedProducer {
  public:
    struct Config {};
    using Parameters = Table<Config>;
    explicit ReplicatedRNG(Parameters const& p,
                           art::ProcessingFrame const& frame)
      : ReplicatedProducer{p, frame}
    {
      auto const id = frame.scheduleID().id();
      assert(createEngine(id).getSeed() == id);
    }

  private:
    void
    produce(art::Event&, art::ProcessingFrame const&) override
    {}
  };
}

DEFINE_ART_MODULE(ReplicatedRNG)
