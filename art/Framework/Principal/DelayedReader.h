#ifndef art_Framework_Principal_DelayedReader_h
#define art_Framework_Principal_DelayedReader_h
// vim: set sw=2 expandtab :

//
// DelayedReader
//
// Abstract interface used by EventPrincipal to request
// input sources to retrieve EDProducts from external storage.
//

#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <vector>

namespace art {

  class Group;
  class Principal;
  class ProductProvenance;
  class RangeSet;

  class DelayedReader {
  public:
    virtual ~DelayedReader() noexcept;
    DelayedReader();

    std::unique_ptr<EDProduct> getProduct(Group const*,
                                          ProductID,
                                          RangeSet&) const;
    void setPrincipal(cet::exempt_ptr<Principal>);
    std::vector<ProductProvenance> readProvenance() const;
    bool isAvailableAfterCombine(ProductID) const;
    std::unique_ptr<Principal> readFromSecondaryFile(int& idx);

  private:
    virtual std::unique_ptr<EDProduct> getProduct_(Group const*,
                                                   ProductID,
                                                   RangeSet&) const = 0;
    virtual void setPrincipal_(cet::exempt_ptr<Principal>);
    virtual std::vector<ProductProvenance> readProvenance_() const;
    virtual bool isAvailableAfterCombine_(ProductID) const;
    virtual std::unique_ptr<Principal> readFromSecondaryFile_(int& idx);
  };

} // namespace art

#endif /* art_Framework_Principal_DelayedReader_h */

// Local Variables:
// mode: c++
// End:
