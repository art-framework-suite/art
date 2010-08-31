#ifndef DataFormats_Common_RefItemGet_h
#define DataFormats_Common_RefItemGet_h

/*----------------------------------------------------------------------

RefItemGet: Free function to get pointer to a referenced item.



----------------------------------------------------------------------*/
#include "art/Persistency/Common/RefCore.h"
#include "art/Persistency/Common/RefItem.h"
#include "art/Persistency/Common/RefCoreGet.h"

namespace edm {

  namespace refitem {
    template< typename C, typename T, typename F, typename KEY>
    struct GetPtrImpl {
      static T const* getPtr_(RefCore const& product, RefItem<KEY> const& item) {
        C const* prod = edm::template getProduct<C>(product);
        /*
        typename C::const_iterator it = prod->begin();
         std::advance(it, item.key());
         T const* p = it.operator->();
        */
        F func;
        T const* p = func(*prod, item.key());
        return p;
      }
    };
  }

  template <typename C, typename T, typename F, typename KEY>
  inline
  T const* getPtr_(RefCore const& product, RefItem<KEY> const& item) {
    return refitem::GetPtrImpl<C, T, F, KEY>::getPtr_(product, item);
  }

  template <typename C, typename T, typename F, typename KEY>
  inline
  T const* getPtr(RefCore const& product, RefItem<KEY> const& item) {
    T const* p = static_cast<T const *>(item.ptr());
    if(0==p){
      p=getPtr_<C, T, F>(product,item);
      item.setPtr(p);
    }
    return p;
  }
}

#endif
