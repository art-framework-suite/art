#ifndef Integration_HierarchicalEDProducer_h
#define Integration_HierarchicalEDProducer_h

/** \class HierarchicalEDProducer
 *
 * \version   1st Version Apr. 6, 2005

 *
 ************************************************************/

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/EDProducer.h"
#include "FWCore/Integration/test/HierarchicalAlgorithms.h"


namespace arttest {
  class HierarchicalEDProducer : public art::EDProducer {
  public:

    explicit HierarchicalEDProducer(art::ParameterSet const& ps);

    virtual ~HierarchicalEDProducer();

    virtual void produce(art::Event& e, art::EventSetup const& c);

  private:
    double       radius_;
    alg_1        outer_alg_;
  };
}
#endif
