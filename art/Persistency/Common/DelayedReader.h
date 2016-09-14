#ifndef art_Persistency_Common_DelayedReader_h
#define art_Persistency_Common_DelayedReader_h
// vim: set sw=2:

//
// DelayedReader
//
// Abstract interface used by EventPrincipal to request
// input sources to retrieve EDProducts from external storage.
//

#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/BranchID.h"
#include "art/Utilities/fwd.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

namespace art {

  class BranchID;
  class BranchKey;
  class EDProductGetterFinder;
  class RangeSet;

  class DelayedReader {

  public:

    virtual ~DelayedReader();

    std::unique_ptr<EDProduct>
    getProduct(BranchKey const& k,
               TypeID const& wrapper_type,
               RangeSet& rs) const
    {
      return getProduct_(k, wrapper_type, rs);
    }

    void
    setGroupFinder(cet::exempt_ptr<EDProductGetterFinder const> ep)
    {
      setGroupFinder_(ep);
    }

    int
    openNextSecondaryFile(int idx)
    {
      return openNextSecondaryFile_(idx);
    }

  private:

    virtual
    std::unique_ptr<EDProduct>
    getProduct_(BranchKey const&,
                TypeID const&,
                RangeSet&) const = 0;

    virtual
    void
    setGroupFinder_(cet::exempt_ptr<EDProductGetterFinder const>);

    virtual
    int
    openNextSecondaryFile_(int idx);

  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Persistency_Common_DelayedReader_h */
