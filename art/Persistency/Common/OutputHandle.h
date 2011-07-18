#ifndef art_Persistency_Common_OutputHandle_h
#define art_Persistency_Common_OutputHandle_h

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

#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "cetlib/exception.h"
#include "cpp0x/memory"

namespace art {
  class EDProduct;
  class OutputHandle {
  public:
    OutputHandle() :
      wrap_(0),
      desc_(0),
      productProvenance_(),
      whyFailed_(){}

    OutputHandle(EDProduct const* prod, ConstBranchDescription const* desc, std::shared_ptr<ProductProvenance> productProvenance) :
      wrap_(prod),
      desc_(desc),
      productProvenance_(productProvenance),
      whyFailed_(){}

    ///Used when the attempt to get the data failed
    OutputHandle(std::shared_ptr<cet::exception> const& iWhyFailed):
      wrap_(0),
      desc_(0),
      productProvenance_(),
      whyFailed_(iWhyFailed) {}

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    void swap(OutputHandle& other) {
      using std::swap;
      std::swap(wrap_, other.wrap_);
      std::swap(desc_, other.desc_);
      std::swap(productProvenance_, other.productProvenance_);
      swap(whyFailed_,other.whyFailed_);
    }

    bool isValid() const {
      return wrap_ && desc_ &&productProvenance_;
    }

    bool failedToGet() const {
      return 0 != whyFailed_.get();
    }

    EDProduct const* wrapper() const {
      return wrap_;
    }

    std::shared_ptr<cet::exception> whyFailed() const {
      return whyFailed_;
    }

    ProductProvenance const* productProvenance() const {
      return productProvenance_.get();
    }

    ConstBranchDescription const* desc() const {
      return desc_;
    }

  private:
    EDProduct const* wrap_;
    ConstBranchDescription const* desc_;
    std::shared_ptr<ProductProvenance> productProvenance_;
    std::shared_ptr<cet::exception> whyFailed_;
  };

  // Free swap function
  inline
  void
  swap(OutputHandle& a, OutputHandle& b) {
    a.swap(b);
  }
}

#endif /* art_Persistency_Common_OutputHandle_h */

// Local Variables:
// mode: c++
// End:
