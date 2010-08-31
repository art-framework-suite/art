#ifndef FWCore_Integration_HistProducer_h
#define FWCore_Integration_HistProducer_h

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDProducer.h"

namespace edmtest {
  //struct ThingWithHist {
//	TH1F hist_;
 // };

  class HistProducer : public edm::EDProducer {
  public:

    explicit HistProducer(edm::ParameterSet const& ps);

    virtual ~HistProducer();

    virtual void produce(edm::Event& e, edm::EventSetup const& c);

  private:
  };
}
#endif
