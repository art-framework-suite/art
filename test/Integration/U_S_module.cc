#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "test/TestObjects/ToyProducts.h"

namespace arttest {
   class U_S;
}

class arttest::U_S : public art::EDProducer {
public:
   explicit U_S(fhicl::ParameterSet const &) {
      produces<IntProduct>();
   }
   virtual ~U_S() {}

   virtual void produce(art::Event &e) {
      std::auto_ptr<IntProduct> p(new IntProduct(1));
      e.put(p);
   }
   void endJob() {}
};

DEFINE_ART_MODULE(arttest::U_S);
