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

#include "test/TestObjects/ToyProducts.h"

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

    ~ToyProductProducer() = default;

    void beginRun(art::Run& r) override
    {
      r.put(std::make_unique<StringProduct>("empty"), RangeSet::for_run(r.id()));
      r.put(std::make_unique<StringProduct>("beginRun"), "bgnRun", RangeSet::for_run(r.id()));
    }

    void beginSubRun(art::SubRun& sr) override
    {
      sr.put(std::make_unique<StringProduct>("emptyAgain"), RangeSet::for_subrun(sr.id()));
      sr.put(std::make_unique<StringProduct>("beginSubRun"), "bgnSubRun", RangeSet::for_subrun(sr.id()));
    }

    void produce(art::Event& e) override
    {
      e.put(std::make_unique<StringProduct>("event"));
    }

    void endSubRun(art::SubRun& sr, art::RangeSet const&) override
    {
      sr.put(std::make_unique<StringProduct>("endSubRun"), "endSubRun", RangeSet::for_subrun(sr.id()));
    }

    void endRun(art::Run& r, art::RangeSet const&) override
    {
      r.put(std::make_unique<StringProduct>("endRun"), "endRun", RangeSet::for_run(r.id()) );
    }

  };  // ToyProductProducer

}

DEFINE_ART_MODULE(arttest::ToyProductProducer)
