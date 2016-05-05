//--------------------------------------------------------------------
//
// Produces an InputProducer instance.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

#include "art/test/TestObjects/ToyProducts.h"

namespace fhicl { class ParameterSet; }

namespace arttest {

  class InputProducer : public art::EDProducer {
  public:

    explicit InputProducer(fhicl::ParameterSet const&)
    {
      produces<StringProduct>();
      produces<StringProduct,art::InSubRun>("endSubRun");
      produces<StringProduct,art::InRun   >("endRun"   );
    }

    virtual void endRun(art::Run& r) override
    {
      r.put(std::make_unique<StringProduct>("endRun"), "endRun", art::runFragment());
    }

    virtual void endSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<StringProduct>("endSubRun"), "endSubRun", art::subRunFragment());
    }

    virtual void produce(art::Event& e) override
    {
      e.put(std::make_unique<StringProduct>("event"));
    }

  };

}

DEFINE_ART_MODULE(arttest::InputProducer)
