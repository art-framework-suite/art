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
  template< typename T > class Wrapper;

}  // art

// ======================================================================

#endif /* art_Persistency_Common_fwd_h */

// Local Variables:
// mode: c++
// End:
