// ======================================================================
// Produces an IntArray instance.
// ======================================================================

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/test/TestObjects/ToyProducts.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

namespace {
  constexpr std::size_t sz{4u};
}

namespace arttest {
  class IntArrayProducer;
}

class arttest::IntArrayProducer : public art::EDProducer {
public:
  struct Config {
  };
  using Parameters = Table<Config>;
  explicit IntArrayProducer(Parameters const&) { produces<IntArray<sz>>(); }

  void
  produce(art::Event& e) override
  {
    int const value = e.id().event();
    auto p = std::make_unique<IntArray<sz>>();
    for (int k = 0; k != sz; ++k) {
      p->arr[k] = value + k;
    }
    e.put(move(p));
  }

}; // IntArrayProducer

DEFINE_ART_MODULE(arttest::IntArrayProducer)
