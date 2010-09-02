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
    produces<ThingCollection, edm::InSubRun>("beginSubRun");
    produces<ThingCollection, edm::InSubRun>("endSubRun");
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

  // Functions that gets called by framework every subRun
  void ThingSource::beginSubRun(edm::SubRun& lb) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into subRun
    lb.put(result, "beginSubRun");
  }

  void ThingSource::endSubRun(edm::SubRun& lb) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into subRun
    lb.put(result, "endSubRun");
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
