#include "FWCore/Integration/test/ThingProducer.h"
#include "test/TestObjects/ThingCollection.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/MakerMacros.h"

namespace arttest {
  ThingProducer::ThingProducer(art::ParameterSet const& iConfig):
  alg_(iConfig.getUntrackedParameter<int>("offsetDelta",0)), //this really should be tracked, but I want backwards compatibility
  noPut_(iConfig.getUntrackedParameter<bool>("noPut", false)) // used for testing with missing products
  {
    produces<ThingCollection>();
    produces<ThingCollection, art::InSubRun>("beginSubRun");
    produces<ThingCollection, art::InSubRun>("endSubRun");
    produces<ThingCollection, art::InRun>("beginRun");
    produces<ThingCollection, art::InRun>("endRun");
  }

  // Virtual destructor needed.
  ThingProducer::~ThingProducer() { }

  // Functions that gets called by framework every event
  void ThingProducer::produce(art::Event& e, art::EventSetup const&) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    if (!noPut_) e.put(result);
  }

  // Functions that gets called by framework every subRun
  void ThingProducer::beginSubRun(art::SubRun& lb, art::EventSetup const&) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into subRun
    if (!noPut_) lb.put(result, "beginSubRun");
  }

  void ThingProducer::endSubRun(art::SubRun& lb, art::EventSetup const&) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into subRun
    if (!noPut_) lb.put(result, "endSubRun");
  }

  // Functions that gets called by framework every run
  void ThingProducer::beginRun(art::Run& r, art::EventSetup const&) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    if (!noPut_) r.put(result, "beginRun");
  }

  void ThingProducer::endRun(art::Run& r, art::EventSetup const&) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    if (!noPut_) r.put(result, "endRun");
  }

}
using arttest::ThingProducer;
DEFINE_FWK_MODULE(ThingProducer);
