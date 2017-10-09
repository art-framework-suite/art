//--------------------------------------------------------------------
//
// Produces an RunSubRunProducerNoPut instance.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

namespace art {
  class Event;
}
namespace fhicl {
  class ParameterSet;
}

namespace arttest {

  class RunSubRunProducerNoPut : public art::EDProducer {
  public:
    explicit RunSubRunProducerNoPut(fhicl::ParameterSet const&)
    {
      produces<int, art::InRun>("bgnRun");
      produces<int, art::InSubRun>("bgnSubRun");
    }

    ~RunSubRunProducerNoPut() override = default;

    // We are not putting the products on the Run or SubRun -- i.e. no
    // overrides for beginRun and beginSubRun.

    void
    produce(art::Event&) override
    {}
  };
}

DEFINE_ART_MODULE(arttest::RunSubRunProducerNoPut)
