#include "art/Persistency/Provenance/Provenance.h"

/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

namespace art {

  Provenance::Provenance(BranchDescription const& p, ProductID const& pid) :
    branchDescription_(p),
    productID_(pid),
    productProvenancePtr_() {
  }

  Provenance::Provenance(ConstBranchDescription const& p, ProductID const& pid) :
    branchDescription_(p),
    productID_(pid),
    productProvenancePtr_() {
  }

  Provenance::Provenance(BranchDescription const& p, ProductID const& pid, std::shared_ptr<ProductProvenance> ei) :
    branchDescription_(p),
    productID_(pid),
    productProvenancePtr_(ei)
  { }

  Provenance::Provenance(ConstBranchDescription const& p, ProductID const& pid, std::shared_ptr<ProductProvenance> ei) :
    branchDescription_(p),
    productID_(pid),
    productProvenancePtr_(ei)
  { }

  void
  Provenance::setProductProvenance(std::shared_ptr<ProductProvenance> bei) const {
    assert(productProvenancePtr_.get() == 0);
    productProvenancePtr_ = bei;
  }

  std::shared_ptr<ProductProvenance>
  Provenance::resolve () const {
    std::shared_ptr<ProductProvenance> prov = store_->branchToProductProvenance(branchDescription_.branchID());
    setProductProvenance(prov);
    return prov;
}


  void
  Provenance::write(std::ostream& os) const {
    // This is grossly inadequate, but it is not critical for the
    // first pass.
    product().write(os);
    productProvenance().write(os);
  }

}

