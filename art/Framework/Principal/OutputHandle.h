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

    OutputHandle() = default;

    OutputHandle(EDProduct const* prod,
                 BranchDescription const* desc,
                 cet::exempt_ptr<ProductProvenance const> productProvenance,
                 bool const rsIDisSet)
      : wrap_{prod}
      , desc_{desc}
      , productProvenance_{productProvenance}
      , rangeSetIDIsSet_{rsIDisSet}
    {}

    ///Used when the attempt to get the data failed
    OutputHandle(std::shared_ptr<cet::exception> const& iWhyFailed)
      : whyFailed_(iWhyFailed)
    {}

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    void swap(OutputHandle& other) {
      using std::swap;
      swap(wrap_, other.wrap_);
      swap(desc_, other.desc_);
      swap(productProvenance_, other.productProvenance_);
      swap(whyFailed_,other.whyFailed_);
    }

    bool isValid() const { return wrap_ && desc_ &&productProvenance_; }
    bool failedToGet() const { return 0 != whyFailed_.get(); }
    bool rangeSetIDIsSet() const { return rangeSetIDIsSet_; }

    EDProduct const* wrapper() const { return wrap_; }

    std::shared_ptr<cet::exception> whyFailed() const { return whyFailed_; }

    ProductProvenance const* productProvenance() const { return productProvenance_.get(); }

    BranchDescription const* desc() const { return desc_; }

  private:
    EDProduct const* wrap_ {nullptr};
    BranchDescription const* desc_ {nullptr};
    cet::exempt_ptr<ProductProvenance const> productProvenance_ {nullptr};
    std::shared_ptr<cet::exception> whyFailed_ {nullptr};
    bool rangeSetIDIsSet_ {false};
  };

  // Free swap function
  inline
  void
  swap(OutputHandle& a, OutputHandle& b) {
    a.swap(b);
  }
}

#endif /* art_Framework_Principal_OutputHandle_h */

// Local Variables:
// mode: c++
// End:
