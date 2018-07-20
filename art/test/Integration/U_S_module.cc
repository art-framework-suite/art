#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/test/TestObjects/ToyProducts.h"

namespace arttest {
  class U_S;
}

class arttest::U_S : public art::EDProducer {
public:
  struct Config {
  };
  using Parameters = Table<Config>;
  explicit U_S(Parameters const& ps) : EDProducer{ps}
  {
    produces<IntProduct>();
  }

private:
  void
  produce(art::Event& e) override
  {
    e.put(std::make_unique<IntProduct>(1));
  }
};

DEFINE_ART_MODULE(arttest::U_S)
