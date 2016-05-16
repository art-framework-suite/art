#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/test/TestObjects/ToyProducts.h"

namespace arttest {
   class U_S;
}

class arttest::U_S : public art::EDProducer {
public:
   explicit U_S(fhicl::ParameterSet const &) {
      produces<IntProduct>();
   }

   void produce(art::Event &e) override {
      std::unique_ptr<IntProduct> p(new IntProduct(1));
      e.put(std::move(p));
   }
   void endJob() override {}
};

DEFINE_ART_MODULE(arttest::U_S)
