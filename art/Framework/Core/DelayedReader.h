#ifndef art_Framework_Core_DelayedReader_h
#define art_Framework_Core_DelayedReader_h

/*----------------------------------------------------------------------

DelayedReader: The abstract interface through which the EventPrincipal
uses input sources to retrieve EDProducts from external storage.

----------------------------------------------------------------------*/

#include <memory>
#include "boost/shared_ptr.hpp"
#include "art/Persistency/Common/EDProduct.h"

namespace art {
  class BranchKey;
  class EDProductGetter;
  class DelayedReader {
  public:
    virtual ~DelayedReader();
    std::auto_ptr<EDProduct> getProduct(BranchKey const& k, EDProductGetter const* ep) {
      return getProduct_(k, ep);
    }
    void mergeReaders(boost::shared_ptr<DelayedReader> other) {mergeReaders_(other);}
  private:
    virtual std::auto_ptr<EDProduct> getProduct_(BranchKey const& k, EDProductGetter const* ep) const = 0;
    virtual void mergeReaders_(boost::shared_ptr<DelayedReader>) {}
  };
}

#endif /* art_Framework_Core_DelayedReader_h */

// Local Variables:
// mode: c++
// End:
