#include "FWCore/Integration/test/OtherThingAlgorithm.h"
#include "art/Framework/Core/Event.h"
#include "test/TestObjects/OtherThing.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/RefToPtr.h"

namespace edmtest {
  void OtherThingAlgorithm::run(art::Event const& event,
				OtherThingCollection& result,
				std::string const& thingLabel,
				std::string const& instance,
				bool refsAreTransient) {

    const size_t numToMake = 20;
    result.reserve(numToMake);
    art::Handle<ThingCollection> parentHandle;
    assert(event.getByLabel(thingLabel, instance, parentHandle));
    assert(parentHandle.isValid());
    ThingCollection const* parent = parentHandle.product();
    ThingCollection const* null = 0;

    for (size_t i = 0; i < numToMake; ++i) {
      OtherThing element;
      element.a = i;
      if (refsAreTransient) {
	element.refProd = art::RefProd<ThingCollection>(parent);
	element.ref = art::Ref<ThingCollection>(element.refProd, i);
	element.refVec.push_back(element.ref);
	element.refVec.push_back(element.ref);
	element.refVec.push_back(art::Ref<ThingCollection>(parent, 19-i));
	art::RefVector<ThingCollection>::iterator ri = element.refVec.begin();
	element.refVec.erase(ri);
	element.oneNullOneNot.push_back(art::Ref<ThingCollection>(null, 0));
	element.oneNullOneNot.push_back(art::Ref<ThingCollection>(parent, 0));
	assert(element.oneNullOneNot.size() == 2); // we'll check this in our tests
	element.ptr = art::Ptr<Thing>(parent, i);
	assert (element.ptr == art::refToPtr(element.ref));
	element.ptrVec.push_back(element.ptr);
	element.ptrVec.push_back(art::Ptr<Thing>(parent, 19-i));
	element.ptrOneNullOneNot.push_back(art::Ptr<Thing>(null, 0));
	element.ptrOneNullOneNot.push_back(art::Ptr<Thing>(parent, 0));
	assert(element.ptrOneNullOneNot.size() == 2); // we'll check this in our tests
	art::RefProd<ThingCollection> refProd = art::RefProd<ThingCollection>(parentHandle);
	art::Ref<ThingCollection> ref = art::Ref<ThingCollection>(refProd, i);
	element.refToBaseProd = art::RefToBaseProd<Thing>(refProd);
	element.refToBase = art::RefToBase<Thing>(ref);
      } else {
	element.refProd = art::RefProd<ThingCollection>(parentHandle);
	element.ref = art::Ref<ThingCollection>(element.refProd, i);
	element.refVec.push_back(element.ref);
	element.refVec.push_back(element.ref);
	element.refVec.push_back(art::Ref<ThingCollection>(parentHandle, 19-i));
	art::RefVector<ThingCollection>::iterator ri = element.refVec.begin();
	element.refVec.erase(ri);
	element.oneNullOneNot.push_back(art::Ref<ThingCollection>(parentHandle.id()));
	element.oneNullOneNot.push_back(art::Ref<ThingCollection>(parentHandle, 0));
	assert(element.oneNullOneNot.size() == 2); // we'll check this in our tests
	element.ptr = art::Ptr<Thing>(parentHandle, i);
	assert (element.ptr == art::refToPtr(element.ref));
	element.ptrVec.push_back(element.ptr);
	element.ptrVec.push_back(art::Ptr<Thing>(parentHandle, 19-i));
	element.ptrOneNullOneNot.push_back(art::Ptr<Thing>(parentHandle.id()));
	element.ptrOneNullOneNot.push_back(art::Ptr<Thing>(parentHandle, 0));
	assert(element.ptrOneNullOneNot.size() == 2); // we'll check this in our tests
	art::RefProd<ThingCollection> refProd = art::RefProd<ThingCollection>(parentHandle);
	art::Ref<ThingCollection> ref = art::Ref<ThingCollection>(refProd, i);
	element.refToBaseProd = art::RefToBaseProd<Thing>(element.refProd);
	element.refToBase = art::RefToBase<Thing>(element.ref);
      }
      result.push_back(element);
      //      element.refVec.clear(); // no need to clear; 'element' is created anew on every loop
    }
  }
}
