#include "TH1F.h"

#include "FWCore/Integration/test/HistProducer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/MakerMacros.h"

namespace edmtest {
  HistProducer::HistProducer(art::ParameterSet const& iConfig)
  {
    produces<TH1F>();
    //produces<ThingWithHist>();
  }

  // Virtual destructor needed.
  HistProducer::~HistProducer() { }

  // Functions that gets called by framework every event
  void HistProducer::produce(art::Event& e, art::EventSetup const&) {

    std::auto_ptr<TH1F> result(new TH1F);  //Empty
    e.put(result);
    //std::auto_ptr<ThingWithHist> result2(new ThingWithHist);  //Empty
    //e.put(result2);
  }

}
using edmtest::HistProducer;
DEFINE_FWK_MODULE(HistProducer);
