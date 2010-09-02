#include "FWCore/Integration/test/ThingProducer.h"
#include "test/TestObjects/ThingCollection.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/MakerMacros.h"

namespace edmtest {
  ThingProducer::ThingProducer(edm::ParameterSet const& iConfig):
  alg_(iConfig.getUntrackedParameter<int>("offsetDelta",0)), //this really should be tracked, but I want backwards compatibility
  noPut_(iConfig.getUntrackedParameter<bool>("noPut", false)) // used for testing with missing products
  {
    produces<ThingCollection>();
    produces<ThingCollection, edm::InSubRun>("beginLumi");
    produces<ThingCollection, edm::InSubRun>("endLumi");
    produces<ThingCollection, edm::InRun>("beginRun");
    produces<ThingCollection, edm::InRun>("endRun");
  }

  // Virtual destructor needed.
  ThingProducer::~ThingProducer() { }

  // Functions that gets called by framework every event
  void ThingProducer::produce(edm::Event& e, edm::EventSetup const&) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    if (!noPut_) e.put(result);
  }

  // Functions that gets called by framework every luminosity block
  void ThingProducer::beginSubRun(edm::SubRun& lb, edm::EventSetup const&) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into lumi block
    if (!noPut_) lb.put(result, "beginLumi");
  }

  void ThingProducer::endSubRun(edm::SubRun& lb, edm::EventSetup const&) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into lumi block
    if (!noPut_) lb.put(result, "endLumi");
  }

  // Functions that gets called by framework every run
  void ThingProducer::beginRun(edm::Run& r, edm::EventSetup const&) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    if (!noPut_) r.put(result, "beginRun");
  }

  void ThingProducer::endRun(edm::Run& r, edm::EventSetup const&) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    if (!noPut_) r.put(result, "endRun");
  }

}
using edmtest::ThingProducer;
DEFINE_FWK_MODULE(ThingProducer);
