#ifndef art_Framework_Principal_Provenance_h
#define art_Framework_Principal_Provenance_h
// vim: set sw=2 expandtab :


//
// FIXME: This class is nothing more than a facade around Group,
//        BranchDescription, and ProductProvenance.
//
// Provenance: The full description of a product and how it came into
//             existence.
//
// definitions:
// Product: The EDProduct to which a provenance object is associated
// Creator: The EDProducer that made the product.
// Parents: The EDProducts used as input by the creator.
//
// Used by Handle and ValidHandle for the most part.
//


#include "art/Framework/Principal/Group.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
//#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/Parentage.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSetID.h"

#include <iosfwd>

namespace art {

class Provenance {

public: // MEMBER FUNCTIONS -- Special Member Functions

  ~Provenance();

  explicit
  Provenance();

  explicit
  Provenance(cet::exempt_ptr<Group const> g);

public: // MEMBER FUNCTIONS -- Full product description

  BranchDescription const&
  productDescription() const;

public: // MEMBER FUNCTIONS -- Selected components of the product description

  std::string const&
  branchName() const;

  std::string const&
  producedClassName() const;

  std::string const&
  friendlyClassName() const;

  std::string const&
  moduleLabel() const;

  std::string const&
  productInstanceName() const;

  std::string const&
  processName() const;

  InputTag
  inputTag() const;

public: // MEMBER FUNCTIONS -- Metadata about the product's origin

  RangeSet const&
  rangeOfValidity() const;

  Parentage const&
  parentage() const;

  std::vector<ProductID> const&
  parents() const;

  fhicl::ParameterSet const&
  parameterSet() const;

  std::set<fhicl::ParameterSetID> const&
  psetIDs() const;

public: // MEMBER FUNCTIONS -- Identifiers corresponding to this product, necessary for Ptr support.

  ProductID const&
  productID() const;

public: // MEMBER FUNCTIONS -- Functions for querying the validity/presence of a product.

  bool
  isValid() const;

  bool
  isPresent() const;

  bool
  produced() const;

  ProductStatus const&
  productStatus() const;

public: // MEMBER FUNCTIONS -- General utilities

  bool
  equals(Provenance const&) const;

  std::ostream&
  write(std::ostream&) const;

private: // MEMBER DATA

  cet::exempt_ptr<Group const>
  group_{nullptr};

};

//void swap(Provenance& x, Provenance& y);

bool
operator==(Provenance const& a, Provenance const& b);

std::ostream&
operator<<(std::ostream& os, Provenance const& p);

} // namespace art

#endif /* art_Framework_Principal_Provenance_h */

// Local Variables:
// mode: c++
// End:
