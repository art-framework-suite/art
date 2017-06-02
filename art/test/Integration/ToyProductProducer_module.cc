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

    struct Config {};
    using Parameters = art::EDProducer::Table<Config>;

    explicit ToyProductProducer(Parameters const&)
    {
      produces<StringProduct,art::InRun>();
      produces<StringProduct,art::InRun>("bgnRun");
      produces<StringProduct,art::InRun>("endRun");

      produces<StringProduct,art::InSubRun>();
      produces<StringProduct,art::InSubRun>("bgnSubRun");
      produces<StringProduct,art::InSubRun>("endSubRun");

      produces<StringProduct>();
    }

  private:

    void beginRun(art::Run& r) override
    {
      r.put(std::make_unique<StringProduct>("empty"), art::fullRun());
      r.put(std::make_unique<StringProduct>("beginRun"), "bgnRun", art::fullRun());
    }

    void beginSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<StringProduct>("emptyAgain"), art::fullSubRun());
      sr.put(std::make_unique<StringProduct>("beginSubRun"), "bgnSubRun", art::fullSubRun());
    }

    void produce(art::Event& e) override
    {
      e.put(std::make_unique<StringProduct>("event"));
    }

    void endSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<StringProduct>("endSubRun"), "endSubRun", art::fullSubRun());
    }

    void endRun(art::Run& r) override
    {
      r.put(std::make_unique<StringProduct>("endRun"), "endRun", art::fullRun() );
    }

  };  // ToyProductProducer

}

DEFINE_ART_MODULE(arttest::ToyProductProducer)
