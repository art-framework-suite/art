#ifndef art_Persistency_Provenance_ProductProvenance_h
#define art_Persistency_Provenance_ProductProvenance_h

/*----------------------------------------------------------------------

ProductProvenance: The event dependent portion of the description of a product
and how it came into existence, plus the status.

----------------------------------------------------------------------*/
#include <iosfwd>
#include <vector>

#include "cpp0x/memory"

#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/ParentageID.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "art/Persistency/Provenance/Transient.h"

/*
  ProductProvenance
*/

namespace art {
  class ProductProvenance {
  public:
    typedef std::vector<ProductProvenance> EntryInfoVector;
    ProductProvenance();
    explicit ProductProvenance(BranchID const& bid);
    ProductProvenance(BranchID const& bid,
		    ProductStatus status);
    ProductProvenance(BranchID const& bid,
		    ProductStatus status,
		    std::shared_ptr<Parentage> parentagePtr);
    ProductProvenance(BranchID const& bid,
		    ProductStatus status,
		    ParentageID const& id);

    ProductProvenance(BranchID const& bid,
		   ProductStatus status,
		   std::vector<BranchID> const& parents);

    ~ProductProvenance() {}

    ProductProvenance makeProductProvenance() const;

    void write(std::ostream& os) const;

    BranchID const& branchID() const {return branchID_;}
    ProductStatus const& productStatus() const {return productStatus_;}
    ParentageID const& parentageID() const {return parentageID_;}
    Parentage const& parentage() const;
    void setStatus(ProductStatus status) {productStatus_ = status;}
    void setPresent();
    void setNotPresent();

    bool & noParentage() const {return transients_.get().noParentage_;}

    struct Transients {
      Transients();
      std::shared_ptr<Parentage> parentagePtr_;
      bool noParentage_;
    };

  private:

    std::shared_ptr<Parentage> & parentagePtr() const {return transients_.get().parentagePtr_;}

    BranchID branchID_;
    ProductStatus productStatus_;
    ParentageID parentageID_;
    mutable Transient<Transients> transients_;
  };

  inline
  bool
  operator < (ProductProvenance const& a, ProductProvenance const& b) {
    return a.branchID() < b.branchID();
  }

  inline
  std::ostream&
  operator<<(std::ostream& os, ProductProvenance const& p) {
    p.write(os);
    return os;
  }

  // Only the 'salient attributes' are testing in equality comparison.
  bool operator==(ProductProvenance const& a, ProductProvenance const& b);
  inline bool operator!=(ProductProvenance const& a, ProductProvenance const& b) { return !(a==b); }
  typedef std::vector<ProductProvenance> ProductProvenanceVector;
}
#endif /* art_Persistency_Provenance_ProductProvenance_h */

// Local Variables:
// mode: c++
// End:
