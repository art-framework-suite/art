#ifndef art_Persistency_Common_DelayedReader_h
#define art_Persistency_Common_DelayedReader_h
// vim: set sw=2:

//
// DelayedReader
//
// Abstract interface used by EventPrincipal to request
// input sources to retrieve EDProducts from external storage.
//

#include "art/Persistency/Common/EDProduct.h"
#include "art/Utilities/fwd.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"

namespace art {

class BranchKey;
class EventPrincipal;
class DelayedReader;

class DelayedReader {

public:

  virtual
  ~DelayedReader();

  std::unique_ptr<EDProduct>
  getProduct(BranchKey const& k, TypeID const& wrapper_type) const
  {
    return getProduct_(k, wrapper_type);
  }

  void
  setGroupFinder(cet::exempt_ptr<EventPrincipal const> ep)
  {
    setGroupFinder_(ep);
  }

  void
  mergeReaders(std::shared_ptr<DelayedReader> other)
  {
    mergeReaders_(other);
  }

  int
  openNextSecondaryFile(int idx)
  {
    return openNextSecondaryFile_(idx);
  }

private:

  virtual
  std::unique_ptr<EDProduct>
  getProduct_(BranchKey const& k, TypeID const& wrapper_type) const = 0;

  virtual
  void
  setGroupFinder_(cet::exempt_ptr<EventPrincipal const>);

  virtual
  void
  mergeReaders_(std::shared_ptr<DelayedReader>);

  virtual
  int
  openNextSecondaryFile_(int idx);

};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Persistency_Common_DelayedReader_h */
