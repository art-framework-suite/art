#ifndef TestObjects_OtherThing_h
#define TestObjects_OtherThing_h

#include "art/Persistency/Common/Ref.h"
#include "art/Persistency/Common/RefVector.h"
#include "art/Persistency/Common/RefProd.h"
#include "art/Persistency/Common/RefToBaseProd.h"
#include "art/Persistency/Common/RefToBase.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "test/TestObjects/ThingCollectionfwd.h"
#include "test/TestObjects/Thing.h"

namespace edmtest {

  struct OtherThing {
    int                             a;
    edm::RefProd<ThingCollection>   refProd;
    edm::Ref<ThingCollection>       ref;
    edm::RefVector<ThingCollection> refVec;
    edm::RefVector<ThingCollection> oneNullOneNot;
    edm::RefToBase<Thing>           refToBase;
    edm::RefToBaseProd<Thing>       refToBaseProd;
    edm::Ptr<Thing>                 ptr;
    edm::PtrVector<Thing>           ptrVec;
    edm::PtrVector<Thing>           ptrOneNullOneNot;
  };
}

#endif
