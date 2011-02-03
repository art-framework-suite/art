#ifndef art_Persistency_Provenance_Provenance_h
#define art_Persistency_Provenance_Provenance_h

/*----------------------------------------------------------------------

Provenance: The full description of a product and how it came into
existence.

----------------------------------------------------------------------*/

#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/ConstBranchDescription.h"
#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "boost/shared_ptr.hpp"
#include "fhiclcpp/ParameterSetID.h"
#include <iosfwd>

// ----------------------------------------------------------------------

/*
  Provenance

  definitions:
  Product: The EDProduct to which a provenance object is associated

  Creator: The EDProducer that made the product.

  Parents: The EDProducts used as input by the creator.
*/

namespace art {
  class Provenance {
  public:
    explicit Provenance(ConstBranchDescription const& p, ProductID const& pid);
    explicit Provenance(BranchDescription const& p, ProductID const& pid);
    Provenance(ConstBranchDescription const& p, ProductID const& pid, boost::shared_ptr<ProductProvenance> entryDesc);
    Provenance(BranchDescription const& p, ProductID const& pid, boost::shared_ptr<ProductProvenance> entryDesc);

    ~Provenance() {}

    Parentage const& event() const {return parentage();}
    BranchDescription const& product() const {return branchDescription_.me();}

    BranchDescription const& branchDescription() const {return branchDescription_.me();}
    ConstBranchDescription const& constBranchDescription() const {return branchDescription_;}
    ProductProvenance const* productProvenancePtr() const {return productProvenancePtr_.get();}
    boost::shared_ptr<ProductProvenance> productProvenanceSharedPtr() const {return productProvenancePtr_;}
    boost::shared_ptr<ProductProvenance> resolve() const;
    ProductProvenance const& productProvenance() const {
      if (productProvenancePtr_.get()) return *productProvenancePtr_;
      return *resolve();
    }
    Parentage const& parentage() const {return productProvenance().parentage();}
    BranchID const& branchID() const {return product().branchID();}
    std::string const& branchName() const {return product().branchName();}
    std::string const& className() const {return product().className();}
    std::string const& moduleLabel() const {return product().moduleLabel();}
    std::string const& processName() const {return product().processName();}
    ProductStatus const& productStatus() const {return productProvenance().productStatus();}
    std::string const& productInstanceName() const {return product().productInstanceName();}
    std::string const& friendlyClassName() const {return product().friendlyClassName();}
    std::set<fhicl::ParameterSetID> const& psetIDs() const {return product().psetIDs();}
    std::set<std::string> const& branchAliases() const {return product().branchAliases();}
    bool isPresent() const {return productstatus::present(productStatus());}

    std::vector<BranchID> const& parents() const {return parentage().parents();}

    void write(std::ostream& os) const;

    void setProductProvenance(boost::shared_ptr<ProductProvenance> bei) const;

    void setStore(boost::shared_ptr<BranchMapper> store) const {store_ = store;}

    ProductID const& productID() const {return productID_;}

  private:
    ConstBranchDescription const branchDescription_;
    ProductID productID_;
    mutable boost::shared_ptr<ProductProvenance> productProvenancePtr_;
    mutable boost::shared_ptr<BranchMapper> store_;
  };

  inline
  std::ostream&
  operator<<(std::ostream& os, Provenance const& p) {
    p.write(os);
    return os;
  }

  bool operator==(Provenance const& a, Provenance const& b);

}  // art

// ======================================================================

#endif /* art_Persistency_Provenance_Provenance_h */

// Local Variables:
// mode: c++
// End:
