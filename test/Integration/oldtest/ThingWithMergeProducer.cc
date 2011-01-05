
//
//
// Puts some simple test objects in the event, run, and subRun
// principals.  The values put into these objects are just
// arbitrary and meaningless.  Later we check that what get
// out when reading the file after merging is what is expected.
//
// Original Author: David Dagenhart, Fermilab, February 2008

#include "FWCore/Integration/test/ThingWithMergeProducer.h"
#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/SubRun.h"
#include "art/Framework/Core/Run.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "test/TestObjects/Thing.h"
#include "test/TestObjects/ThingWithMerge.h"
#include "test/TestObjects/ThingWithIsEqual.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Utilities/InputTag.h"
#include "art/ParameterSet/ParameterSet.h"

namespace arttest {
  ThingWithMergeProducer::ThingWithMergeProducer(art::ParameterSet const& pset) :
    changeIsEqualValue_(pset.getUntrackedParameter<bool>("changeIsEqualValue", false)),
    labelsToGet_(pset.getUntrackedParameter<std::vector<std::string> >("labelsToGet", std::vector<std::string>())),
    noPut_(pset.getUntrackedParameter<bool>("noPut", false))
{
    produces<Thing>("event");
    produces<Thing, art::InSubRun>("beginSubRun");
    produces<Thing, art::InSubRun>("endSubRun");
    produces<Thing, art::InRun>("beginRun");
    produces<Thing, art::InRun>("endRun");

    produces<ThingWithMerge>("event");
    produces<ThingWithMerge, art::InSubRun>("beginSubRun");
    produces<ThingWithMerge, art::InSubRun>("endSubRun");
    produces<ThingWithMerge, art::InRun>("beginRun");
    produces<ThingWithMerge, art::InRun>("endRun");

    produces<ThingWithIsEqual>("event");
    produces<ThingWithIsEqual, art::InSubRun>("beginSubRun");
    produces<ThingWithIsEqual, art::InSubRun>("endSubRun");
    produces<ThingWithIsEqual, art::InRun>("beginRun");
    produces<ThingWithIsEqual, art::InRun>("endRun");
  }

  ThingWithMergeProducer::~ThingWithMergeProducer() {}

  void ThingWithMergeProducer::produce(art::Event& e, art::EventSetup const&) {

    // The purpose of this first getByLabel call is to cause the products
    // that are "put" below to have a parent so we can do tests with the
    // parentage provenance.
    for (Iter iter = labelsToGet_.begin(), ie = labelsToGet_.end(); iter != ie; ++iter) {
      art::Handle<Thing> h;
      art::InputTag tag(*iter, "event", "PROD");
      e.getByLabel(tag, h);
    }

    std::auto_ptr<Thing> result(new Thing);
    result->a = 11;
    if (!noPut_) e.put(result, std::string("event"));

    std::auto_ptr<ThingWithMerge> result2(new ThingWithMerge);
    result2->a = 12;
    if (!noPut_) e.put(result2, std::string("event"));

    std::auto_ptr<ThingWithIsEqual> result3(new ThingWithIsEqual);
    result3->a = 13;
    if (changeIsEqualValue_) result3->a = 14;
    if (!noPut_) e.put(result3, std::string("event"));
  }

  void ThingWithMergeProducer::beginSubRun(art::SubRun& lb, art::EventSetup const&) {

    for (Iter iter = labelsToGet_.begin(), ie = labelsToGet_.end(); iter != ie; ++iter) {
      art::Handle<Thing> h;
      art::InputTag tag(*iter, "beginSubRun", "PROD");
      lb.getByLabel(tag, h);
    }

    std::auto_ptr<Thing> result(new Thing);
    result->a = 101;
    if (!noPut_) lb.put(result, "beginSubRun");

    std::auto_ptr<ThingWithMerge> result2(new ThingWithMerge);
    result2->a = 102;
    if (!noPut_) lb.put(result2, "beginSubRun");

    std::auto_ptr<ThingWithIsEqual> result3(new ThingWithIsEqual);
    result3->a = 103;
    if (changeIsEqualValue_) result3->a = 104;
    if (!noPut_) lb.put(result3, "beginSubRun");
  }

  void ThingWithMergeProducer::endSubRun(art::SubRun& lb, art::EventSetup const&) {

    for (Iter iter = labelsToGet_.begin(), ie = labelsToGet_.end(); iter != ie; ++iter) {
      art::Handle<Thing> h;
      art::InputTag tag(*iter, "endSubRun", "PROD");
      lb.getByLabel(tag, h);
    }

    std::auto_ptr<Thing> result(new Thing);
    result->a = 1001;
    if (!noPut_) lb.put(result, "endSubRun");

    std::auto_ptr<ThingWithMerge> result2(new ThingWithMerge);
    result2->a = 1002;
    if (!noPut_) lb.put(result2, "endSubRun");

    std::auto_ptr<ThingWithIsEqual> result3(new ThingWithIsEqual);
    result3->a = 1003;
    if (changeIsEqualValue_) result3->a = 1004;
    if (!noPut_) lb.put(result3, "endSubRun");
  }

  // Functions that gets called by framework every run
  void ThingWithMergeProducer::beginRun(art::Run& r, art::EventSetup const&) {

    for (Iter iter = labelsToGet_.begin(), ie = labelsToGet_.end(); iter != ie; ++iter) {
      art::Handle<Thing> h;
      art::InputTag tag(*iter, "beginRun", "PROD");
      r.getByLabel(tag, h);
    }

    std::auto_ptr<Thing> result(new Thing);
    result->a = 10001;
    if (!noPut_) r.put(result, "beginRun");

    std::auto_ptr<ThingWithMerge> result2(new ThingWithMerge);
    result2->a = 10002;
    if (!noPut_) r.put(result2, "beginRun");

    std::auto_ptr<ThingWithIsEqual> result3(new ThingWithIsEqual);
    result3->a = 10003;
    if (changeIsEqualValue_) result3->a = 10004;
    if (!noPut_) r.put(result3, "beginRun");
  }

  void ThingWithMergeProducer::endRun(art::Run& r, art::EventSetup const&) {

    for (Iter iter = labelsToGet_.begin(), ie = labelsToGet_.end(); iter != ie; ++iter) {
      art::Handle<Thing> h;
      art::InputTag tag(*iter, "endRun", "PROD");
      r.getByLabel(tag, h);
    }

    std::auto_ptr<Thing> result(new Thing);
    result->a = 100001;
    if (!noPut_) r.put(result, "endRun");

    std::auto_ptr<ThingWithMerge> result2(new ThingWithMerge);
    result2->a = 100002;
    if (!noPut_) r.put(result2, "endRun");

    std::auto_ptr<ThingWithIsEqual> result3(new ThingWithIsEqual);
    result3->a = 100003;
    if (changeIsEqualValue_) result3->a = 100004;
    if (!noPut_) r.put(result3, "endRun");
  }
}
using arttest::ThingWithMergeProducer;
DEFINE_FWK_MODULE(ThingWithMergeProducer);
