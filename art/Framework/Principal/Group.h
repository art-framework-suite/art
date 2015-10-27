#ifndef art_Framework_Principal_Group_h
#define art_Framework_Principal_Group_h
// vim: set sw=2:

//
//  A collection of information related to a single EDProduct.
//

#include "art/Framework/Principal/GroupFactory.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/BranchMapper.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Utilities/fwd.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

namespace art {

//
// Noncopyable through inheritance from EDProductGetter.
//

class Group : public EDProductGetter {

#ifndef __GCCXML__

  friend
  std::unique_ptr<Group>
  gfactory::
  make_group(BranchDescription const&, ProductID const&);

  friend
  std::unique_ptr<Group>
  gfactory::
  make_group(BranchDescription const&, ProductID const&,
             cet::exempt_ptr<Worker>, cet::exempt_ptr<EventPrincipal>);

  friend
  std::unique_ptr<Group>
  gfactory::
  make_group(std::unique_ptr<EDProduct>&&, BranchDescription const&,
             ProductID const&);

#endif // __GCCXML__

public:

  Group();

protected:

  //
  // Use GroupFactory to make.
  //

  Group(BranchDescription const& bd, ProductID const& pid,
        TypeID const& wrapper_type,
        cet::exempt_ptr<Worker> productProducer = cet::exempt_ptr<Worker>(),
        cet::exempt_ptr<EventPrincipal> onDemandPrincipal =
          cet::exempt_ptr<EventPrincipal>());

#ifndef __GCCXML__

  Group(std::unique_ptr<EDProduct>&& edp, BranchDescription const& bd,
        ProductID const& pid, TypeID const& wrapper_type);

#endif // __GCCXML__

public:

  void swap(Group& other);

  // product is not available (dropped or never created)
  bool productUnavailable() const;

  // Scheduled for on-demand production
  bool onDemand() const
  {
    return productProducer_ && onDemandPrincipal_;
  }

  bool isReady() const override
  {
    return true;
  }

  EDProduct const* getIt() const override
  {
    resolveProductIfAvailable(true, producedWrapperType());
    return product_.get();
  }

#ifndef __GCCXML__

  EDProduct const* anyProduct() const override
  {
    return product_.get();
  }

  EDProduct const* uniqueProduct() const override
  {
    return product_.get();
  }

  EDProduct const* uniqueProduct(TypeID const&) const override
  {
    return product_.get();
  }

#endif // __GCCXML__

  cet::exempt_ptr<ProductProvenance const> productProvenancePtr() const;

  BranchDescription const& productDescription() const
  {
    return *branchDescription_;
  }

  std::string const& moduleLabel() const
  {
    return branchDescription_->moduleLabel();
  }

  std::string const& productInstanceName() const
  {
    return branchDescription_->productInstanceName();
  }

  std::string const& processName() const
  {
    return branchDescription_->processName();
  }

  ProductStatus status() const;

  void setResolvers(BranchMapper const& bm, DelayedReader const& dr)
  {
    ppResolver_.reset(&bm);
    productResolver_.reset(&dr);
  }

  virtual bool resolveProduct(bool fillOnDemand, TypeID const&) const;

  virtual bool resolveProductIfAvailable(bool fillOnDemand,
                                         TypeID const&) const;
  // Write the group to the stream.
  void write(std::ostream& os) const;

  // Replace the existing group with a new one
  void replace(Group& g);

  ProductID const& productID() const
  {
    return pid_;
  };

  TypeID const& producedWrapperType() const
  {
    return wrapper_type_;
  }

  // Remove any cached product.
  void removeCachedProduct() const;

protected:

#ifndef __GCCXML__

  std::unique_ptr<EDProduct> obtainDesiredProduct(bool fillOnDemand,
                                                  TypeID const&) const;

  void setProduct(std::unique_ptr<EDProduct>&& prod) const;

#endif // __GCCXML__

private:

  bool dropped() const;

private:

  TypeID wrapper_type_;
  cet::exempt_ptr<BranchMapper const> ppResolver_;
  cet::exempt_ptr<DelayedReader const> productResolver_;
#ifndef __GCCXML__
  mutable std::unique_ptr<EDProduct> product_;
#endif // __GCCXML__
  cet::exempt_ptr<BranchDescription const> branchDescription_;
  mutable ProductID pid_;
  cet::exempt_ptr<Worker> productProducer_;
  // FIXME: This will be a generic principal when meta data is fixed.
  cet::exempt_ptr<EventPrincipal> onDemandPrincipal_;
};  // Group

#ifndef __GCCXML__
inline
std::ostream&
operator<<(std::ostream& os, Group const& g)
{
  g.write(os);
  return os;
}
#endif // __GCCXML__

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_Group_h */
