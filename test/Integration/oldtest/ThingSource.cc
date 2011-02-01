#include "FWCore/Integration/test/ThingSource.h"
#include "test/TestObjects/ThingCollection.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/InputSourceMacros.h"

namespace arttest {
  ThingSource::ThingSource(art::ParameterSet const& pset, art::InputSourceDescription const& desc) :
    GeneratedInputSource(pset, desc), alg_() {
    produces<ThingCollection>();
    produces<ThingCollection, art::InSubRun>("beginSubRun");
    produces<ThingCollection, art::InSubRun>("endSubRun");
    produces<ThingCollection, art::InRun>("beginRun");
    produces<ThingCollection, art::InRun>("endRun");
  }

  // Virtual destructor needed.
  ThingSource::~ThingSource() { }

  // Functions that gets called by framework every event
  bool ThingSource::produce(art::Event& e) {
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
  void ThingSource::beginSubRun(art::SubRun& sr) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into subRun
    sr.put(result, "beginSubRun");
  }

  void ThingSource::endSubRun(art::SubRun& sr) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into subRun
    sr.put(result, "endSubRun");
  }

  // Functions that gets called by framework every run
  void ThingSource::beginRun(art::Run& r) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    r.put(result, "beginRun");
  }

  void ThingSource::endRun(art::Run& r) {
    // Step A: Get Inputs

    // Step B: Create empty output
    std::auto_ptr<ThingCollection> result(new ThingCollection);  //Empty

    // Step C: Invoke the algorithm, passing in inputs (NONE) and getting back outputs.
    alg_.run(*result);

    // Step D: Put outputs into event
    r.put(result, "endRun");
  }
}
using arttest::ThingSource;
DEFINE_ART_INPUT_SOURCE(ThingSource);
