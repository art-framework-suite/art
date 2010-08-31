#ifndef DataFormats_Common_MapRefViewTrait_h
#define DataFormats_Common_MapRefViewTrait_h
#include "art/Persistency/Common/Ref.h"
#include "art/Persistency/Common/RefProd.h"
#include "art/Persistency/Common/RefToBase.h"
#include "art/Persistency/Common/RefToBaseProd.h"
#include <map>

namespace edm {
  namespace helper {
    template<typename C>
    struct MapRefViewTrait {
      typedef Ref<C> ref_type;
      typedef RefProd<C> refprod_type;
    };

    template<typename T>
    struct MapRefViewTrait<View<T> > {
      typedef RefToBase<T> ref_type;
      typedef RefToBaseProd<T> refprod_type;
    };
  }
}

#endif
