#ifndef art_Persistency_Common_DelayedReader_h
#define art_Persistency_Common_DelayedReader_h
// vim: set sw=2 expandtab :

//
// DelayedReader
//
// Abstract interface used by EventPrincipal to request
// input sources to retrieve EDProducts from external storage.
//

#include "art/Utilities/fwd.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

namespace art {

  class Group;
  class Principal;
  class ProductProvenance;
  struct BranchKey;
  class RangeSet;

  class DelayedReader {

    // MEMBER FUNCTIONS -- Special Member Functions
  public:
    virtual ~DelayedReader() noexcept;
    DelayedReader();

    // MEMBER FUNCTIONS -- API
  public:
    std::unique_ptr<EDProduct> getProduct(Group const*,
                                          ProductID,
                                          TypeID const& wrapper_type,
                                          RangeSet&) const;
    void setPrincipal(cet::exempt_ptr<Principal>);
    std::vector<ProductProvenance> readProvenance() const;
    bool isAvailableAfterCombine(ProductID) const;
    int openNextSecondaryFile(int idx);

    // MEMBER FUNCTIONS -- Implementation details.
  private:
    virtual std::unique_ptr<EDProduct> getProduct_(Group const*,
                                                   ProductID,
                                                   TypeID const&,
                                                   RangeSet&) const = 0;
    virtual void setPrincipal_(cet::exempt_ptr<Principal>);
    virtual std::vector<ProductProvenance> readProvenance_() const;
    virtual bool isAvailableAfterCombine_(ProductID) const;
    virtual int openNextSecondaryFile_(int idx);
  };

} // namespace art

#endif /* art_Persistency_Common_DelayedReader_h */

// Local Variables:
// mode: c++
// End:
