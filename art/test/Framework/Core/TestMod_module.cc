#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"

class TestMod : public art::EDProducer {
public:
  explicit TestMod(fhicl::ParameterSet const&) {}

private:
  void
  produce(art::Event&) override
  {}
};

DEFINE_ART_MODULE(TestMod)
