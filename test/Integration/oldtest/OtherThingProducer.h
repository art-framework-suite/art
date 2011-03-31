#ifndef test_Integration_oldtest_OtherThingProducer_h
#define test_Integration_oldtest_OtherThingProducer_h

#include <string>
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDProducer.h"
#include "FWCore/Integration/test/OtherThingAlgorithm.h"

namespace arttest {
  class OtherThingProducer : public art::EDProducer {
  public:

    // The following is not yet used, but will be the primary
    // constructor when the parameter set system is available.
    //
    explicit OtherThingProducer(fhicl::ParameterSet const& ps);

    virtual ~OtherThingProducer();

    virtual void produce(art::Event& e, art::EventSetup const& c);

  private:
    OtherThingAlgorithm alg_;
    std::string thingLabel_;
    bool refsAreTransient_;
  };
}

#endif /* test_Integration_oldtest_OtherThingProducer_h */

// Local Variables:
// mode: c++
// End:
