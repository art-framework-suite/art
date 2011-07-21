// ======================================================================
//
// Provenance: The full description of a product and how it came into
//             existence.
//
// definitions:
// Product: The EDProduct to which a provenance object is associated
// Creator: The EDProducer that made the product.
// Parents: The EDProducts used as input by the creator.
//
// ======================================================================

#include "art/Persistency/Provenance/Provenance.h"

using art::Provenance;

#if 0
void
Provenance::setProductProvenance(std::shared_ptr<ProductProvenance> bei) const {
  assert(group_->productProvenancePtr().get() == 0);
  productProvenancePtr_ = bei;
}

std::shared_ptr<art::ProductProvenance>
Provenance::resolve () const {
  std::shared_ptr<ProductProvenance> prov = store_->branchToProductProvenance(product().branchID());
  setProductProvenance(prov);
  return prov;
}
#endif  // 0

std::ostream &
Provenance::write(std::ostream& os) const {
  // This is grossly inadequate, but it is not critical for the
  // first pass.
  product().write(os);
  productProvenance().write(os);
  return os;
}

// ======================================================================
