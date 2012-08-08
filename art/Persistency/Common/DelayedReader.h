#ifndef art_Persistency_Common_DelayedReader_h
#define art_Persistency_Common_DelayedReader_h

/*----------------------------------------------------------------------

DelayedReader: The abstract interface through which the EventPrincipal
uses input sources to retrieve EDProducts from external storage.

----------------------------------------------------------------------*/

#include "art/Persistency/Common/EDProduct.h"
#include "art/Utilities/fwd.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"

namespace art {
  class BranchKey;
  // Use this instead of Principal/fwd.h to prevent false positives in dependency checking.
  class EventPrincipal;

  class DelayedReader;
}

class art::DelayedReader {
public:
  virtual ~DelayedReader();
  std::unique_ptr<EDProduct> getProduct(BranchKey const& k, art::TypeID const &wrapper_type) const;
  void setGroupFinder(cet::exempt_ptr<EventPrincipal const>);
  void mergeReaders(std::shared_ptr<DelayedReader> other) {mergeReaders_(other);}
private:
  virtual std::unique_ptr<EDProduct> getProduct_(BranchKey const& k, art::TypeID const &wrapper_type) const = 0;
  virtual void setGroupFinder_(cet::exempt_ptr<EventPrincipal const>);
  virtual void mergeReaders_(std::shared_ptr<DelayedReader>);
};

inline
std::unique_ptr<art::EDProduct>
art::DelayedReader::
getProduct(BranchKey const& k, art::TypeID const &wrapper_type) const {
  return getProduct_(k, wrapper_type);
}

inline
void
art::DelayedReader::
setGroupFinder(cet::exempt_ptr<EventPrincipal const> ep) {
  setGroupFinder_(ep);
}

#endif /* art_Persistency_Common_DelayedReader_h */

// Local Variables:
// mode: c++
// End:
