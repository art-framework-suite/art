#ifndef FWCore_Integration_HistProducer_h
#define FWCore_Integration_HistProducer_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDProducer.h"

namespace arttest {
  //struct ThingWithHist {
//	TH1F hist_;
 // };

  class HistProducer : public art::EDProducer {
  public:

    explicit HistProducer(art::ParameterSet const& ps);

    virtual ~HistProducer();

    virtual void produce(art::Event& e, art::EventSetup const& c);

  private:
  };
}
#endif
