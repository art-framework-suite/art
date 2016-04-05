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

#include "art/Framework/Principal/Provenance.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"

fhicl::ParameterSet const &
art::Provenance::parameterSet() const
{
  return fhicl::ParameterSetRegistry::get(*psetIDs().begin());
}

art::RangeSet const &
art::Provenance::rangeSet() const
{
  auto p = group_->productResolver();
  assert(p != nullptr);
  return p->getRangeSet(branchID());
}

std::ostream &
art::Provenance::write(std::ostream& os) const
{
  // This is grossly inadequate, but it is not critical for the
  // first pass.
  productDescription().write(os);
  productProvenance().write(os);
  return os;
}

// ======================================================================
