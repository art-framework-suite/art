#ifndef test_Integration_oldtest_ThingProducer_h
#define test_Integration_oldtest_ThingProducer_h

/** \class ThingProducer
 *
 * \version   1st Version Apr. 6, 2005

 *
 ************************************************************/

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDProducer.h"
#include "FWCore/Integration/test/ThingAlgorithm.h"

namespace arttest {
  class ThingProducer : public art::EDProducer {
  public:

    explicit ThingProducer(art::ParameterSet const& ps);

    virtual ~ThingProducer();

    virtual void produce(art::Event& e, art::EventSetup const& c);

    virtual void beginRun(art::Run& r, art::EventSetup const& c);

    virtual void endRun(art::Run& r, art::EventSetup const& c);

    virtual void beginSubRun(art::SubRun& sr, art::EventSetup const& c);

    virtual void endSubRun(art::SubRun& sr, art::EventSetup const& c);

  private:
    ThingAlgorithm alg_;
    bool noPut_;
  };
}
#endif /* test_Integration_oldtest_ThingProducer_h */

// Local Variables:
// mode: c++
// End:
