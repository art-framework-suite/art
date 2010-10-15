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

  Provenance::Provenance(BranchDescription const& p, ProductID const& pid, boost::shared_ptr<ProductProvenance> ei) :
    branchDescription_(p),
    productID_(pid),
    productProvenancePtr_(ei)
  { }

  Provenance::Provenance(ConstBranchDescription const& p, ProductID const& pid, boost::shared_ptr<ProductProvenance> ei) :
    branchDescription_(p),
    productID_(pid),
    productProvenancePtr_(ei)
  { }

  void
  Provenance::setProductProvenance(boost::shared_ptr<ProductProvenance> bei) const {
    assert(productProvenancePtr_.get() == 0);
    productProvenancePtr_ = bei;
  }

  boost::shared_ptr<ProductProvenance>
  Provenance::resolve () const {
    boost::shared_ptr<ProductProvenance> prov = store_->branchToEntryInfo(branchDescription_.branchID());
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


  bool operator==(Provenance const& a, Provenance const& b) {
    return
      a.product() == b.product()
      && a.productProvenance() == b.productProvenance();
  }

}

