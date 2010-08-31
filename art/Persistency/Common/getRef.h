#ifndef DataFormats_Common_getRef_h
#define DataFormats_Common_getRef_h
/* \function edm::getRef(...)
 *
 * \author Luca Lista, INFN
 *
 */
#include "art/Persistency/Common/Ref.h"
#include "art/Persistency/Common/View.h"
#include "art/Persistency/Common/Handle.h"

namespace edm {
  namespace helper {
    template<typename C>
    struct MatcherGetRef {
      typedef Ref<C> ref_type;
      static ref_type getRef( const Handle<C> & c, size_t k ) { return ref_type(c, k); }
    };

    template<typename T>
    struct MatcherGetRef<View<T> > {
      typedef RefToBase<T> ref_type;
      static ref_type getRef( const Handle<View<T> > & v, size_t k ) { return v->refAt(k); }
    };
  }

  template<typename C>
  typename helper::MatcherGetRef<C>::ref_type getRef( const Handle<C> & c, size_t k ) {
    return helper::MatcherGetRef<C>::getRef( c, k );
  }
}

#endif
