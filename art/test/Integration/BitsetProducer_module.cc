// ======================================================================
// Produces an std::bitset<4> instance.
// ======================================================================

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/SharedProducer.h"
#include "art/Framework/Principal/Event.h"
#include "fhiclcpp/ParameterSet.h"

#include <bitset>
#include <memory>

namespace {
  constexpr std::size_t sz{4u};
}

namespace art::test {
  class BitsetProducer : public SharedProducer {
  public:
    struct Config {};
    using Parameters = Table<Config>;
    explicit BitsetProducer(Parameters const& p, ProcessingFrame const&)
      : SharedProducer{p}
    {
      async<InEvent>();
      produces<std::bitset<sz>>();
    }

  private:
    void
    produce(Event& e, ProcessingFrame const&) override
    {
      e.put(std::make_unique<std::bitset<sz>>(0b1011));
    }
  };
}

DEFINE_ART_MODULE(art::test::BitsetProducer)
