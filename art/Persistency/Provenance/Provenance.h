#ifndef art_Persistency_Provenance_Provenance_h
#define art_Persistency_Provenance_Provenance_h

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

#include "art/Framework/Core/Group.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSetID.h"
#include <iosfwd>

namespace art {
  class Provenance;
  std::ostream &
    operator << ( std::ostream &, Provenance const & );
}

// ----------------------------------------------------------------------

class art::Provenance
{
public:
  Provenance( ) : group_( )  { }
  Provenance( cet::exempt_ptr<Group const> g ) : group_( g )  { }

  // use compiler-generated copy c'tor, copy assignment, and d'tor

  bool  isValid( ) const  { return group_; }

  BranchDescription const&  product            () const {return group_->productDescription();}
  BranchDescription const&  branchDescription  () const {return product();}
  ProductID         const&  productID          () const {return group_->productID();}
  Parentage         const&  event              () const {return parentage();}
  Parentage         const&  parentage          () const {return productProvenance().parentage();}
  BranchID          const&  branchID           () const {return product().branchID();}
  std::string       const&  branchName         () const {return product().branchName();}
  std::string       const&  className          () const {return product().className();}
  std::string       const&  moduleLabel        () const {return product().moduleLabel();}
  std::string       const&  processName        () const {return product().processName();}
  ProductStatus     const&  productStatus      () const {return productProvenance().productStatus();}
  std::string       const&  productInstanceName() const {return product().productInstanceName();}
  std::string       const&  friendlyClassName  () const {return product().friendlyClassName();}
  std::set<fhicl::ParameterSetID> const&
                            psetIDs            () const {return product().psetIDs();}
  std::set<std::string> const&
                            branchAliases      () const {return product().branchAliases();}
  std::vector<BranchID> const&
                            parents            () const {return parentage().parents();}

  bool isPresent()  const {return productstatus::present(productStatus());}

  std::ostream & write(std::ostream& os) const;

private:
  cet::exempt_ptr<Group const> group_;

  ProductProvenance const*  productProvenancePtr() const {return group_->productProvenancePtr().get();}
  ProductProvenance const&  productProvenance   () const {return * productProvenancePtr();}

};  // Provenance

inline  std::ostream &
  art::operator << ( std::ostream & os, Provenance const & p )
{
  return p.write(os);
}

// ======================================================================

#endif /* art_Persistency_Provenance_Provenance_h */

// Local Variables:
// mode: c++
// End:
