#ifndef art_Persistency_Common_EDProductGetter_h
#define art_Persistency_Common_EDProductGetter_h

#include "art/Persistency/Provenance/ProductID.h"

#include "boost/noncopyable.hpp"

namespace art {
  class EDProduct;
  class EDProductGetter;
}

class art::EDProductGetter : private boost::noncopyable {
public:
  virtual ~EDProductGetter();

  virtual EDProduct const *getIt() const = 0;
};  // EDProductGetter

#endif /* art_Persistency_Common_EDProductGetter_h */

// Local Variables:
// mode: c++
// End:
