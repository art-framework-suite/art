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
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/fwd.h"

#include <iosfwd>
#include <set>
#include <string>
#include <vector>

namespace art {

  class Provenance {
  public:
    // Special Member Functions
    explicit Provenance();
    explicit Provenance(cet::exempt_ptr<Group const> g) noexcept;

    // Full product description
    BranchDescription const& productDescription() const noexcept;

    // Selected components of the product description
    std::string const& branchName() const noexcept;
    std::string const& producedClassName() const noexcept;
    std::string const& friendlyClassName() const noexcept;
    std::string const& moduleLabel() const noexcept;
    std::string const& productInstanceName() const noexcept;
    std::string const& processName() const noexcept;
    InputTag inputTag() const;

    // Metadata about the product's origin
    RangeSet const& rangeOfValidity() const;
    Parentage const& parentage() const;
    std::vector<ProductID> const& parents() const;
    fhicl::ParameterSet const& parameterSet() const;
    std::set<fhicl::ParameterSetID> const& psetIDs() const noexcept;

    // Identifiers corresponding to this product, necessary for Ptr
    // support.
    ProductID productID() const noexcept;

    // Functions for querying the validity/presence of a product.
    bool isValid() const noexcept;
    bool isPresent() const;
    bool produced() const noexcept;
    ProductStatus productStatus() const;

    // General utilities
    bool equals(Provenance const&) const noexcept;
    std::ostream& write(std::ostream&) const;

  private:
    ProductProvenance const& productProvenance() const;

    cet::exempt_ptr<Group const> group_{nullptr};
  };

  bool operator==(Provenance const& a, Provenance const& b) noexcept;
  std::ostream& operator<<(std::ostream& os, Provenance const& p);
}
#endif /* art_Framework_Principal_Provenance_h */

// Local Variables:
// mode: c++
// End:
