#ifndef art_Framework_Principal_OutputHandle_h
#define art_Framework_Principal_OutputHandle_h

/*----------------------------------------------------------------------

Handle: Non-owning "smart pointer" for reference to EDProducts and
their Provenances.

This is a very preliminary version, and lacks safety features and
elegance.

If the pointed-to object or provenance destroyed, use of the
Handle becomes undefined. There is no way to query the Handle to
discover if this has happened.

Handles can have:
  -- Product and Provenance pointers both null;
  -- Both pointers valid

To check validity, one can use the isValid() function.

If failedToGet() returns true then the requested data is not available
If failedToGet() returns false but isValid() is also false then no attempt
  to get data has occurred

----------------------------------------------------------------------*/

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "cetlib/exception.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

namespace art {
  class EDProduct;
  class OutputHandle {
  public:
    OutputHandle(EDProduct const* prod,
                 BranchDescription const* desc,
                 cet::exempt_ptr<ProductProvenance const> productProvenance,
                 RangeSet const& rs)
      : wrap_{prod}
      , desc_{desc}
      , productProvenance_{productProvenance}
      , rangeOfValidity_{rs}
    {}

    /// Used when the attempt to get the data failed
    OutputHandle(RangeSet const& rs) : rangeOfValidity_{rs} {}

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    void
    swap(OutputHandle& other)
    {
      using std::swap;
      swap(wrap_, other.wrap_);
      swap(desc_, other.desc_);
      swap(productProvenance_, other.productProvenance_);
    }

    bool
    isValid() const
    {
      return wrap_ && desc_ && productProvenance_;
    }

    RangeSet const&
    rangeOfValidity() const
    {
      return rangeOfValidity_;
    }

    EDProduct const*
    wrapper() const
    {
      return wrap_;
    }

    ProductProvenance const*
    productProvenance() const
    {
      return productProvenance_.get();
    }

    BranchDescription const*
    desc() const
    {
      return desc_;
    }

  private:
    EDProduct const* wrap_{nullptr};
    BranchDescription const* desc_{nullptr};
    cet::exempt_ptr<ProductProvenance const> productProvenance_{nullptr};
    RangeSet const& rangeOfValidity_;
  };

  // Free swap function
  inline void
  swap(OutputHandle& a, OutputHandle& b)
  {
    a.swap(b);
  }
}

#endif /* art_Framework_Principal_OutputHandle_h */

// Local Variables:
// mode: c++
// End:
