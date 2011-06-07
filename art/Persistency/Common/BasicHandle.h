#ifndef art_Persistency_Common_BasicHandle_h
#define art_Persistency_Common_BasicHandle_h

/*----------------------------------------------------------------------

Handle: Non-owning "smart pointer" for reference to EDProducts and
their Provenances.

This is a very preliminary version, and lacks safety features and
elegance.

If the pointed-to EDProduct or Provenance is destroyed, use of the
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

#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/Provenance.h"
#include "cetlib/exception.h"
#include "cpp0x/memory"

// ----------------------------------------------------------------------

namespace art {
  class EDProduct;

  class BasicHandle
  {
  public:
    BasicHandle() :
      product_(),
      prov_(0),
      id_(),
      whyFailed_(){}

    BasicHandle(std::shared_ptr<EDProduct const> prod, Provenance const* prov) :
      product_(prod), prov_(prov), id_(prov->productID()) {
    }

    // Used when the attempt to get the data failed
    BasicHandle(std::shared_ptr<cet::exception> const& iWhyFailed):
    product_(),
    prov_(0),
    id_(),
    whyFailed_(iWhyFailed) {}

    void swap(BasicHandle& other) {
      using std::swap;
      swap(product_, other.product_);
      std::swap(prov_, other.prov_);
      std::swap(id_, other.id_);
      swap(whyFailed_,other.whyFailed_);
    }

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    bool isValid() const {
      return product_ && prov_;
    }

    bool failedToGet() const {
      return 0 != whyFailed_.get();
    }

    EDProduct const* wrapper() const {
      return product_.get();
    }

    std::shared_ptr<EDProduct const> product() const {
      return product_;
    }

    Provenance const* provenance() const {
      return prov_;
    }

    ProductID id() const {
      return id_;
    }

    std::shared_ptr<cet::exception> whyFailed() const {
      return whyFailed_;
    }
  private:
    std::shared_ptr<EDProduct const> product_;
    Provenance const* prov_;
    ProductID id_;
    std::shared_ptr<cet::exception> whyFailed_;
  };  // BasicHandle

  // Free swap function
  inline
  void
  swap(BasicHandle& a, BasicHandle& b) {
    a.swap(b);
  }
}

// ======================================================================

#endif /* art_Persistency_Common_BasicHandle_h */

// Local Variables:
// mode: c++
// End:
