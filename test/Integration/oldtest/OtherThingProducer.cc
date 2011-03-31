#include "FWCore/Integration/test/OtherThingProducer.h"
#include "test/TestObjects/OtherThingCollection.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/ModuleMacros.h"

namespace arttest {
  OtherThingProducer::OtherThingProducer(fhicl::ParameterSet const& pset): alg_(), thingLabel_(), refsAreTransient_(false) {
    produces<OtherThingCollection>("testUserTag");
    thingLabel_ = pset.get<std::string>("thingLabel", std::string("Thing"));
    refsAreTransient_ = pset.get<bool>("transient", false);
  }

  // Virtual destructor needed.
  OtherThingProducer::~OtherThingProducer() {}

  // Functions that gets called by framework every event
  void OtherThingProducer::produce(art::Event& e, art::EventSetup const&) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<OtherThingCollection> result(new OtherThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(e, *result, thingLabel_, std::string(), refsAreTransient_);

    // Step D: Put outputs into event
    e.put(result, std::string("testUserTag"));
  }
}
using arttest::OtherThingProducer;
DEFINE_ART_MODULE(OtherThingProducer);
