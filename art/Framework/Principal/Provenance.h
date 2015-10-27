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
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/Parentage.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "cetlib/exempt_ptr.h"
#include <memory>
#include "fhiclcpp/ParameterSetID.h"
#include <iosfwd>

namespace art {
  class Provenance;
  std::ostream & operator << ( std::ostream &, Provenance const & );
  bool operator==(Provenance const& a, Provenance const& b);
  void swap(Provenance& x, Provenance& y);
}

// ----------------------------------------------------------------------

class art::Provenance
{
public:
  Provenance( ) : group_( )  { }
  Provenance( cet::exempt_ptr<Group const> g ) : group_( g )  { }

  // use compiler-generated copy c'tor, copy assignment, and d'tor

  bool  isValid( ) const  { return static_cast<bool>(group_); }

  BranchDescription   const & productDescription () const {return group_->productDescription();}
  BranchDescription   const & branchDescription  () const {return productDescription();}
  ProductID           const & productID          () const {return group_->productID();}
  Parentage           const & event              () const {return parentage();}
  Parentage           const & parentage          () const {return productProvenance().parentage();}
  BranchID            const & branchID           () const {return productDescription().branchID();}
  std::string         const & branchName         () const {return productDescription().branchName();}
  std::string         const & producedClassName  () const {return productDescription().producedClassName();}
  std::string         const & moduleLabel        () const {return productDescription().moduleLabel();}
  std::string         const & processName        () const {return productDescription().processName();}
  bool                        produced           () const {return productDescription().produced();}
  ProductStatus       const & productStatus      () const {return productProvenance().productStatus();}
  std::string         const & productInstanceName() const {return productDescription().productInstanceName();}
  std::string         const & friendlyClassName  () const {return productDescription().friendlyClassName();}
  fhicl::ParameterSet const & parameterSet       () const;
  std::set<fhicl::ParameterSetID> const &
                            psetIDs            () const {return productDescription().psetIDs();}
  std::vector<BranchID> const&
                            parents            () const {return parentage().parents();}

  bool isPresent()  const {return productstatus::present(productStatus());}

  std::ostream & write(std::ostream& os) const;
  bool equals(Provenance const& other) const { return group_ == other.group_; }

  void swap(Provenance & other) { std::swap(group_, other.group_); }

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

inline bool
art::operator==( art::Provenance const& a, art::Provenance const& b)
{
  return a.equals(b);
}

inline void
art::swap(art::Provenance & x, art::Provenance & y)
{
  x.swap(y);
}

// ======================================================================

#endif /* art_Framework_Principal_Provenance_h */

// Local Variables:
// mode: c++
// End:
