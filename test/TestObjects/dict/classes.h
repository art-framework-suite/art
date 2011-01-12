/* #include "art/Persistency/Common/SortedCollection.h" */
/* #include "art/Persistency/Common/OwnVector.h" */
/* #include "art/Persistency/Common/AssociationVector.h" */
#include "art/Persistency/Common/Wrapper.h"

/* #include "test/TestObjects/OtherThingCollection.h" */
/* #include "test/TestObjects/ThingCollection.h" */
#include "test/TestObjects/ToyProducts.h"
/* #include "test/TestObjects/Thing.h" */
/* #include "test/TestObjects/ThingWithMerge.h" */
/* #include "test/TestObjects/ThingWithIsEqual.h" */

/* #include "test/TestObjects/StreamTestSimple.h" */
/* #include "test/TestObjects/StreamTestThing.h" */
/* #include "test/TestObjects/StreamTestTmpl.h" */

/* #include "art/Persistency/Common/Holder.h" */
/* #include "art/Persistency/Common/RefToBaseProd.h" */

namespace {
struct dictionary {
  art::Wrapper<arttest::DummyProduct> dummyw12;
  art::Wrapper<arttest::IntProduct> dummyw13;
  art::Wrapper<arttest::Int16_tProduct> dummyw23;
  art::Wrapper<arttest::DoubleProduct> dummyw14;
  art::Wrapper<arttest::StringProduct> dummyw15;
/*   art::Wrapper<arttest::SCSimpleProduct> dummyw16; */
/*   art::Wrapper<arttest::OVSimpleProduct> dummyw17; */
/*   art::Wrapper<arttest::OVSimpleDerivedProduct> dummyw17Derived; */
/*   art::Wrapper<arttest::AVSimpleProduct> dummyw18; */
/*   art::Wrapper<arttest::DSVSimpleProduct> dummyw19; */
/*   art::Wrapper<arttest::DSVWeirdProduct> dummyw20; */
/*   art::Wrapper<arttest::DSTVSimpleProduct> dummyw21; */
/*   art::Wrapper<arttest::DSTVSimpleDerivedProduct> dummyw22; */
  art::Wrapper<arttest::Prodigal> dummyw24;

/*   art::Wrapper<arttest::Thing> dummy105; */
/*   art::Wrapper<arttest::ThingWithMerge> dummy104; */
/*   art::Wrapper<arttest::ThingWithIsEqual> dummy103; */

/*   arttest::ThingCollection dummy1; */
/*   arttest::OtherThingCollection dummy2; */
/*   art::Wrapper<arttest::ThingCollection> dummy3; */
/*   art::Wrapper<arttest::OtherThingCollection> dummy4; */

/*   arttestprod::Ord<arttestprod::Simple> dummy20; */
/*   arttestprod::StreamTestTmpl<arttestprod::Ord<arttestprod::Simple> > dummy21; */
/*   art::Wrapper<arttestprod::StreamTestTmpl<arttestprod::Ord<arttestprod::Simple> > > dummy22; */
/*   std::vector<arttestprod::Simple> dummy23; */
/*   std::vector<arttest::Simple> dummy231; */
/*   art::Wrapper<std::vector<arttest::Simple> > dummy231w; */
/*   art::RefProd<std::vector<arttest::Simple> > dummy232; */
/*   art::SortedCollection<arttestprod::Simple,art::StrictWeakOrdering<arttestprod::Simple> > dummy24; */
/*   art::Wrapper<art::SortedCollection<arttestprod::Simple,art::StrictWeakOrdering<arttestprod::Simple> > > dummy25; */
/*   art::Wrapper<arttestprod::StreamTestThing> dummy26; */
/*   art::Wrapper<arttestprod::X0123456789012345678901234567890123456789012345678901234567890123456789012345678901> dummy27; */
/*   art::DetSet<arttest::Sortable> x1; */
/*   art::DetSet<arttest::Unsortable> x2; */
/*   std::vector<arttest::Sortable> x3; */
/*   std::vector<arttest::Unsortable> x4; */

/*   art::reftobase::Holder<arttest::Thing,art::Ref<std::vector<arttest::Thing> > > bhThing; */
/*   art::RefToBaseProd<arttest::Thing> rtbpThing; */

/*   art::Ptr<arttest::Thing> ptrThing; */
/*   art::PtrVector<arttest::Thing> ptrVecThing; */
};
}
