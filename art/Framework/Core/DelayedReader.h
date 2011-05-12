#ifndef art_Framework_Core_DelayedReader_h
#define art_Framework_Core_DelayedReader_h

/*----------------------------------------------------------------------

DelayedReader: The abstract interface through which the EventPrincipal
uses input sources to retrieve EDProducts from external storage.

----------------------------------------------------------------------*/

#include "art/Framework/Core/FCPfwd.h"
#include "art/Persistency/Common/EDProduct.h"
#include "cpp0x/memory"

namespace art {
  class BranchKey;
  class EDProductGetter;
  class DelayedReader {
  public:
    virtual ~DelayedReader();
    std::auto_ptr<EDProduct> getProduct(BranchKey const& k, EDProductGetter const* ep) {
      return getProduct_(k, ep);
    }
    void mergeReaders(std::shared_ptr<DelayedReader> other) {mergeReaders_(other);}
  private:
    virtual std::auto_ptr<EDProduct> getProduct_(BranchKey const& k, EDProductGetter const* ep) const = 0;
    virtual void mergeReaders_(std::shared_ptr<DelayedReader>) {}
  };
}

#endif /* art_Framework_Core_DelayedReader_h */

// Local Variables:
// mode: c++
// End:
