#include "FWCore/Integration/test/ThingExtSource.h"
#include "test/TestObjects/ThingCollection.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/InputSourceMacros.h"

namespace edmtest {
  ThingExtSource::ThingExtSource(edm::ParameterSet const& pset, edm::InputSourceDescription const& desc) :
    ExternalInputSource(pset, desc), alg_() {
    produces<ThingCollection>();
    produces<ThingCollection, edm::InSubRun>("beginSubRun");
    produces<ThingCollection, edm::InSubRun>("endSubRun");
    produces<ThingCollection, edm::InRun>("beginRun");
    produces<ThingCollection, edm::InRun>("endRun");
  }

  // Virtual destructor needed.
  ThingExtSource::~ThingExtSource() { }

  // Functions that gets called by framework every event
  bool ThingExtSource::produce(edm::Event& e) {

    // Fake running out of data for an external input source.
    if (event() > 2) return false;

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
  void ThingExtSource::beginSubRun(edm::SubRun& lb) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into subRun
    lb.put(result, "beginSubRun");
  }

  void ThingExtSource::endSubRun(edm::SubRun& lb) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into subRun
    lb.put(result, "endSubRun");
  }

  // Functions that gets called by framework every run
  void ThingExtSource::beginRun(edm::Run& r) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    r.put(result, "beginRun");
  }

  void ThingExtSource::endRun(edm::Run& r) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    r.put(result, "endRun");
  }

}
using edmtest::ThingExtSource;
DEFINE_FWK_INPUT_SOURCE(ThingExtSource);
