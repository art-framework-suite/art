//--------------------------------------------------------------------
//
// Main motivation for RunProductProducer is to test product
// aggregation.
//
//--------------------------------------------------------------------

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "fhiclcpp/types/Atom.h"

#include "test/TestObjects/ToyProducts.h"

namespace {

  struct Config {
    fhicl::Atom<unsigned> increment { fhicl::Name("increment") };
  };

  class RunProductProducerTiered : public art::EDProducer {
    unsigned const increment_;
    unsigned counter_ {};

  public:

    using Parameters = Table<Config>;
    explicit RunProductProducerTiered(Parameters const& config)
      : increment_{config().increment()}
    {
      produces<unsigned,art::InRun>("counts");
    }

    void produce(art::Event&) override
    {
      counter_ += increment_;
    }

    void endRun(art::Run& r, art::RangeSet const& seen) override
    {
      r.put(std::make_unique<unsigned>(counter_), "counts", seen);
      counter_ = 0u;
    }

  };  // RunProductProducerTiered

}

DEFINE_ART_MODULE(RunProductProducerTiered)
