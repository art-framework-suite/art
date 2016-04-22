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

#include "test/TestObjects/ToyProducts.h"

namespace fhicl { class ParameterSet; }

namespace arttest {

  class InputProducerNoEvents : public art::EDProducer {
  public:

    explicit InputProducerNoEvents(fhicl::ParameterSet const&)
    {
      produces<StringProduct,art::InSubRun>("endSubRun");
      produces<StringProduct,art::InRun   >("endRun"   );
    }

    virtual void endRun(art::Run& r) override
    {
      r.put(std::make_unique<StringProduct>("endRun"), "endRun", r.seenRangeSet());
    }

    virtual void endSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<StringProduct>("endSubRun"), "endSubRun", sr.seenRangeSet());
    }

    virtual void produce(art::Event&) override {}
  };

}

DEFINE_ART_MODULE(arttest::InputProducerNoEvents)
