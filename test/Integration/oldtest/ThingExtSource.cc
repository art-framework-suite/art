#include "FWCore/Integration/test/ThingExtSource.h"
#include "test/TestObjects/ThingCollection.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/InputSourceMacros.h"

namespace arttest {
  ThingExtSource::ThingExtSource(art::ParameterSet const& pset, art::InputSourceDescription const& desc) :
    ExternalInputSource(pset, desc), alg_() {
    produces<ThingCollection>();
    produces<ThingCollection, art::InSubRun>("beginSubRun");
    produces<ThingCollection, art::InSubRun>("endSubRun");
    produces<ThingCollection, art::InRun>("beginRun");
    produces<ThingCollection, art::InRun>("endRun");
  }

  // Virtual destructor needed.
  ThingExtSource::~ThingExtSource() { }

  // Functions that gets called by framework every event
  bool ThingExtSource::produce(art::Event& e) {

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
  void ThingExtSource::beginSubRun(art::SubRun& lb) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into subRun
    lb.put(result, "beginSubRun");
  }

  void ThingExtSource::endSubRun(art::SubRun& lb) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into subRun
    lb.put(result, "endSubRun");
  }

  // Functions that gets called by framework every run
  void ThingExtSource::beginRun(art::Run& r) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    r.put(result, "beginRun");
  }

  void ThingExtSource::endRun(art::Run& r) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    r.put(result, "endRun");
  }

}
using arttest::ThingExtSource;
DEFINE_FWK_INPUT_SOURCE(ThingExtSource);
