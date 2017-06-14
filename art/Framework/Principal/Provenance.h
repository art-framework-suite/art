#ifndef art_Framework_Principal_Provenance_h
#define art_Framework_Principal_Provenance_h

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

#include "art/Framework/Principal/Group.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/Parentage.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSetID.h"

#include <iosfwd>

namespace art {
  class Provenance;
  std::ostream& operator<<(std::ostream&, Provenance const&);
  bool operator==(Provenance const& a, Provenance const& b);
  void swap(Provenance& x, Provenance& y);
}

// ----------------------------------------------------------------------

class art::Provenance {
public:

  explicit constexpr Provenance() = default;
  explicit Provenance(cet::exempt_ptr<Group const> g) : group_{g} {}

  // Full product description
  BranchDescription const& productDescription() const {return group_->productDescription();}

  // Selected components of the product description
  std::string const& branchName() const {return productDescription().branchName();}
  std::string const& producedClassName() const {return productDescription().producedClassName();}
  std::string const& friendlyClassName() const {return productDescription().friendlyClassName();}
  std::string const& moduleLabel() const {return productDescription().moduleLabel();}
  std::string const& productInstanceName() const {return productDescription().productInstanceName();}
  std::string const& processName() const {return productDescription().processName();}
  InputTag inputTag() const {return InputTag{moduleLabel(), productInstanceName(), processName()};}

  // Metadata about the product's origin
  RangeSet const& rangeOfValidity() const {return group_->rangeOfValidity();}
  Parentage const& parentage() const {return productProvenance().parentage();}
  std::vector<ProductID> const& parents() const {return parentage().parents();}
  fhicl::ParameterSet const& parameterSet() const;
  std::set<fhicl::ParameterSetID> const& psetIDs() const {return productDescription().psetIDs();}

  // Identifiers corresponding to this product, necessary for art::Ptr support.
  ProductID const& productID() const {return group_->productID();}

  // Functions for querying the validity/presence of a product.
  bool isValid() const { return static_cast<bool>(group_); }
  bool isPresent() const {return productstatus::present(productStatus());}
  bool produced() const {return productDescription().produced();}
  ProductStatus const& productStatus() const {return productProvenance().productStatus();}

  // General utilities
  std::ostream& write(std::ostream& os) const;
  bool equals(Provenance const& other) const { return group_ == other.group_; }

private:
  cet::exempt_ptr<Group const> group_ {nullptr};

  ProductProvenance const& productProvenance() const { return *group_->productProvenancePtr(); }

};  // Provenance

inline std::ostream&
art::operator<<(std::ostream& os, Provenance const& p)
{
  return p.write(os);
}

inline bool
art::operator==(art::Provenance const& a, art::Provenance const& b)
{
  return a.equals(b);
}

// ======================================================================

#endif /* art_Framework_Principal_Provenance_h */

// Local Variables:
// mode: c++
// End:
