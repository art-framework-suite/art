#include "art/Framework/Core/ModuleMacros.h"
#include "FWCore/Integration/test/HierarchicalEDProducer.h"

namespace arttest {

  HierarchicalEDProducer::HierarchicalEDProducer(fhicl::ParameterSet const& ps) :
    radius_ (ps.get<double>("radius")),
    outer_alg_(ps.get<fhicl::ParameterSet>("nest_1"))
  { produces<int>();}

  // Virtual destructor needed.
  HierarchicalEDProducer::~HierarchicalEDProducer() {}

  // Functions that gets called by framework every event
  void HierarchicalEDProducer::produce(art::Event&, art::EventSetup const&) {
    // nothing to do ... is just a dummy!
  }
}
using arttest::HierarchicalEDProducer;
DEFINE_ART_MODULE(HierarchicalEDProducer);
