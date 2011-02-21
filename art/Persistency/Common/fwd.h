#ifndef art_Persistency_Common_fwd_h
#define art_Persistency_Common_fwd_h

// ======================================================================
//
// Forward declarations of types in Persistency/Common
//
// ======================================================================

#include "boost/shared_ptr.hpp"

namespace art {

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
 //  template< typename C
//           , typename T
//           , typename F > class RefVectorIterator;
  class RefVectorIterator;
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

}  // art

// ======================================================================

#endif /* art_Persistency_Common_fwd_h */

// Local Variables:
// mode: c++
// End:
