#ifndef DataFormats_Common_RefHolder_h
#define DataFormats_Common_RefHolder_h
#include "art/Persistency/Common/RefHolder_.h"

#include "art/Persistency/Common/IndirectVectorHolder.h"
#include "art/Persistency/Common/RefVectorHolder.h"
#include "art/Persistency/Common/RefVector.h"
#include "art/Persistency/Common/HolderToVectorTrait.h"

namespace edm {
  namespace reftobase {
    template <class REF>
    std::auto_ptr<RefVectorHolderBase> RefHolder<REF>::makeVectorHolder() const {
      typedef typename RefHolderToRefVectorTrait<REF>::type helper;
      return helper::makeVectorHolder();
    }
  }
}

#include "art/Persistency/Common/RefKeyTrait.h"

namespace edm {
  namespace reftobase {
    template <class REF>
    size_t
    RefHolder<REF>::key() const
    {
      typedef typename RefKeyTrait<REF>::type helper;
      return helper::key( ref_ );
    }

  }
}

#endif
