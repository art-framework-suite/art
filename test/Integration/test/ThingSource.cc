#include "FWCore/Integration/test/ThingSource.h"
#include "test/TestObjects/ThingCollection.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/InputSourceMacros.h"

namespace edmtest {
  ThingSource::ThingSource(edm::ParameterSet const& pset, edm::InputSourceDescription const& desc) :
    GeneratedInputSource(pset, desc), alg_() {
    produces<ThingCollection>();
    produces<ThingCollection, edm::InSubRun>("beginLumi");
    produces<ThingCollection, edm::InSubRun>("endLumi");
    produces<ThingCollection, edm::InRun>("beginRun");
    produces<ThingCollection, edm::InRun>("endRun");
  }

  // Virtual destructor needed.
  ThingSource::~ThingSource() { }

  // Functions that gets called by framework every event
  bool ThingSource::produce(edm::Event& e) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    e.put(result);

    return true;
  }

  // Functions that gets called by framework every luminosity block
  void ThingSource::beginSubRun(edm::SubRun& lb) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into lumi block
    lb.put(result, "beginLumi");
  }

  void ThingSource::endSubRun(edm::SubRun& lb) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into lumi block
    lb.put(result, "endLumi");
  }

  // Functions that gets called by framework every run
  void ThingSource::beginRun(edm::Run& r) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    r.put(result, "beginRun");
  }

  void ThingSource::endRun(edm::Run& r) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    r.put(result, "endRun");
  }
}
using edmtest::ThingSource;
DEFINE_FWK_INPUT_SOURCE(ThingSource);
