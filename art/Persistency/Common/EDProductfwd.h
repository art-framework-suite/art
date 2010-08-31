#ifndef DataFormats_Common_EDProductfwd_h
#define DataFormats_Common_EDProductfwd_h

// ======================================================================
//
// EDProductfwd: Forward declarations of types in the EDM
//
// ======================================================================


#include "boost/shared_ptr.hpp"


namespace edm {

  class BasicHandle;
  class EDProduct;
  class EDProductGetter;
  class OutputHandle;
  class ProductID;
  class RefCore;

  template< typename T > class Handle;
  template< typename T > class OrphanHandle;
  template< typename C
          , typename T
          , typename F > class Ref;
  template< typename K > class RefBase;
  template< typename K > class RefItem;
  template< typename T > class RefProd;
  template< typename T > class RefToBase;
  template< typename T > class RefToBaseVector;
  template< typename C
          , typename T
          , typename F > class RefVector;
  template< typename T > class RefVectorBase;
  template< typename C
          , typename T
          , typename F > class RefVectorIterator;
  template< typename T > class Wrapper;

  namespace reftobase
  {
    class RefHolderBase;
    class RefVectorHolderBase;
    template< typename T > class BaseHolder;
    template< typename T > class BaseVectorHolder;
    template< typename T
            , typename R > class Holder;
    template< typename T > class IndirectHolder;
    template< typename R > class RefHolder;
    template< typename T
            , typename V > class VectorHolder;
  }
  typedef  boost::shared_ptr<reftobase::RefHolderBase>
           helper_ptr;
  typedef  reftobase::RefVectorHolderBase
           helper_vector;
  typedef  boost::shared_ptr<reftobase::RefVectorHolderBase>
           helper_vector_ptr;

}  // namespace edm


// ======================================================================

#endif  // DataFormats_Common_EDProductfwd_h
