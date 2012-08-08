#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "test/TestObjects/ToyProducts.h"

namespace arttest {
   class AddIntsProducer;
}

class arttest::AddIntsProducer : public art::EDProducer {
public:
   explicit AddIntsProducer(fhicl::ParameterSet const& p) :
      labels_(p.get<std::vector<std::string> >("labels")) {
      produces<IntProduct>();
   }
   virtual ~AddIntsProducer() { }
   virtual void produce(art::Event& e);
private:
   std::vector<std::string> labels_;
};

void
arttest::AddIntsProducer::produce(art::Event& e) {
   int value = 0;
   for(std::vector<std::string>::iterator
          itLabel = labels_.begin(),
          itLabelEnd = labels_.end();
       itLabel != itLabelEnd;
       ++itLabel) {
      art::Handle<IntProduct> anInt;
      e.getByLabel(*itLabel, anInt);
      value +=anInt->value;
   }
   std::unique_ptr<IntProduct> p(new IntProduct(value));
   e.put(std::move(p));
}

DEFINE_ART_MODULE(arttest::AddIntsProducer)
