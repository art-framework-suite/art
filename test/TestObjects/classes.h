#include "art/Persistency/Common/SortedCollection.h"
#include "art/Persistency/Common/OwnVector.h"
#include "art/Persistency/Common/AssociationVector.h"
#include "art/Persistency/Common/Wrapper.h"

#include "test/TestObjects/OtherThingCollection.h"
#include "test/TestObjects/ThingCollection.h"
#include "test/TestObjects/ToyProducts.h"
#include "test/TestObjects/Thing.h"
#include "test/TestObjects/ThingWithMerge.h"
#include "test/TestObjects/ThingWithIsEqual.h"

#include "test/TestObjects/StreamTestSimple.h"
#include "test/TestObjects/StreamTestThing.h"
#include "test/TestObjects/StreamTestTmpl.h"

#include "art/Persistency/Common/Holder.h"
#include "art/Persistency/Common/RefToBaseProd.h"

namespace {
struct dictionary {
  art::Wrapper<edmtest::DummyProduct> dummyw12;
  art::Wrapper<edmtest::IntProduct> dummyw13;
  art::Wrapper<edmtest::DoubleProduct> dummyw14;
  art::Wrapper<edmtest::StringProduct> dummyw15;
  art::Wrapper<edmtest::SCSimpleProduct> dummyw16;
  art::Wrapper<edmtest::OVSimpleProduct> dummyw17;
  art::Wrapper<edmtest::OVSimpleDerivedProduct> dummyw17Derived;
  art::Wrapper<edmtest::AVSimpleProduct> dummyw18;
  art::Wrapper<edmtest::DSVSimpleProduct> dummyw19;
  art::Wrapper<edmtest::DSVWeirdProduct> dummyw20;
  art::Wrapper<edmtest::DSTVSimpleProduct> dummyw21;
  art::Wrapper<edmtest::DSTVSimpleDerivedProduct> dummyw22;
  art::Wrapper<edmtest::Int16_tProduct> dummyw23;
  art::Wrapper<edmtest::Prodigal> dummyw24;

  art::Wrapper<edmtest::Thing> dummy105;
  art::Wrapper<edmtest::ThingWithMerge> dummy104;
  art::Wrapper<edmtest::ThingWithIsEqual> dummy103;

  edmtest::ThingCollection dummy1;
  edmtest::OtherThingCollection dummy2;
  art::Wrapper<edmtest::ThingCollection> dummy3;
  art::Wrapper<edmtest::OtherThingCollection> dummy4;

  edmtestprod::Ord<edmtestprod::Simple> dummy20;
  edmtestprod::StreamTestTmpl<edmtestprod::Ord<edmtestprod::Simple> > dummy21;
  art::Wrapper<edmtestprod::StreamTestTmpl<edmtestprod::Ord<edmtestprod::Simple> > > dummy22;
  std::vector<edmtestprod::Simple> dummy23;
  std::vector<edmtest::Simple> dummy231;
  art::Wrapper<std::vector<edmtest::Simple> > dummy231w;
  art::RefProd<std::vector<edmtest::Simple> > dummy232;
  art::SortedCollection<edmtestprod::Simple,art::StrictWeakOrdering<edmtestprod::Simple> > dummy24;
  art::Wrapper<art::SortedCollection<edmtestprod::Simple,art::StrictWeakOrdering<edmtestprod::Simple> > > dummy25;
  art::Wrapper<edmtestprod::StreamTestThing> dummy26;
  art::Wrapper<edmtestprod::X0123456789012345678901234567890123456789012345678901234567890123456789012345678901> dummy27;
  art::DetSet<edmtest::Sortable> x1;
  art::DetSet<edmtest::Unsortable> x2;
  std::vector<edmtest::Sortable> x3;
  std::vector<edmtest::Unsortable> x4;

  art::reftobase::Holder<edmtest::Thing,art::Ref<std::vector<edmtest::Thing> > > bhThing;
  art::RefToBaseProd<edmtest::Thing> rtbpThing;

  art::Ptr<edmtest::Thing> ptrThing;
  art::PtrVector<edmtest::Thing> ptrVecThing;
};
}
