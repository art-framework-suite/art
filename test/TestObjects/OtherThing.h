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
    art::RefProd<ThingCollection>   refProd;
    art::Ref<ThingCollection>       ref;
    art::RefVector<ThingCollection> refVec;
    art::RefVector<ThingCollection> oneNullOneNot;
    art::RefToBase<Thing>           refToBase;
    art::RefToBaseProd<Thing>       refToBaseProd;
    art::Ptr<Thing>                 ptr;
    art::PtrVector<Thing>           ptrVec;
    art::PtrVector<Thing>           ptrOneNullOneNot;
  };
}

#endif
