//--------------------------------------------------------------------
//
// Produces an ToyProductProducer instance.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"

#include "art/test/TestObjects/ToyProducts.h"

namespace fhicl { class ParameterSet; }

using art::RangeSet;

namespace arttest {

  class ToyProductProducer : public art::EDProducer {
  public:

    explicit ToyProductProducer( fhicl::ParameterSet const& )
    {
      produces<StringProduct,art::InRun>();
      produces<StringProduct,art::InRun>("bgnRun");
      produces<StringProduct,art::InRun>("endRun");

      produces<StringProduct,art::InSubRun>();
      produces<StringProduct,art::InSubRun>("bgnSubRun");
      produces<StringProduct,art::InSubRun>("endSubRun");

      produces<StringProduct>();
    }

    void beginRun(art::Run& r) override
    {
      r.put(std::make_unique<StringProduct>("empty"), FullRun);
      r.put(std::make_unique<StringProduct>("beginRun"), "bgnRun", FullRun);
    }

    void beginSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<StringProduct>("emptyAgain"), FullSubRun);
      sr.put(std::make_unique<StringProduct>("beginSubRun"), "bgnSubRun", FullSubRun);
    }

    void produce(art::Event& e) override
    {
      e.put(std::make_unique<StringProduct>("event"));
    }

    void endSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<StringProduct>("endSubRun"), "endSubRun", FullSubRun);
    }

    void endRun(art::Run& r) override
    {
      r.put(std::make_unique<StringProduct>("endRun"), "endRun", FullRun );
    }

  };  // ToyProductProducer

}

DEFINE_ART_MODULE(arttest::ToyProductProducer)
