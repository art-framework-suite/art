#ifndef art_Framework_Principal_Group_h
#define art_Framework_Principal_Group_h
// vim: set sw=2:

//
//  A collection of information related to a single EDProduct.
//

#include "art/Framework/Principal/GroupFactory.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/EDProductGetter.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "art/Utilities/fwd.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

namespace art {

  //
  // Noncopyable through inheritance from EDProductGetter.
  //

  class Group : public EDProductGetter {

    friend
    std::unique_ptr<Group>
    gfactory::
    make_group(BranchDescription const&,
               ProductID const&,
               RangeSet&&);

    friend
    std::unique_ptr<Group>
    gfactory::
    make_group(BranchDescription const&,
               ProductID const&,
               RangeSet&&,
               cet::exempt_ptr<Worker>,
               cet::exempt_ptr<EventPrincipal>);

    friend
    std::unique_ptr<Group>
    gfactory::
    make_group(std::unique_ptr<EDProduct>&&,
               BranchDescription const&,
               ProductID const&,
               RangeSet&&);

  public:

    Group() = default;

  protected:

    //
    // Use GroupFactory to make.
    //

    Group(BranchDescription const& bd,
          ProductID const& pid,
          TypeID const& wrapper_type,
          RangeSet&& rs,
          cet::exempt_ptr<Worker> productProducer = cet::exempt_ptr<Worker>{},
          cet::exempt_ptr<EventPrincipal> onDemandPrincipal = cet::exempt_ptr<EventPrincipal>{});

    Group(std::unique_ptr<EDProduct>&& edp,
          BranchDescription const& bd,
          ProductID const& pid,
          TypeID const& wrapper_type,
          RangeSet&& rs);

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

    bool resolveProduct(bool fillOnDemand, TypeID const&) const override;

    bool resolveProductIfAvailable(bool fillOnDemand,
                                   TypeID const&) const override;

    void write(std::ostream& os) const;
    void replace(Group& g);

    ProductID const& productID() const
    {
      return pid_;
    };

    TypeID const& producedWrapperType() const
    {
      return wrapper_type_;
    }

    void removeCachedProduct() const;

    RangeSet const& rangeOfValidity() const { return rangeOfValidity_; }

  protected:

    std::unique_ptr<EDProduct> obtainDesiredProduct(bool fillOnDemand,
                                                    TypeID const&) const;

    void setProduct(std::unique_ptr<EDProduct>&& prod) const;

  private:

    bool dropped() const;

  private:

    TypeID wrapper_type_ {};
    cet::exempt_ptr<BranchMapper const> ppResolver_ {nullptr};
    cet::exempt_ptr<DelayedReader const> productResolver_ {nullptr};
    mutable std::unique_ptr<EDProduct> product_ {nullptr};
    cet::exempt_ptr<BranchDescription const> branchDescription_ {nullptr};
    mutable ProductID pid_ {};
    cet::exempt_ptr<Worker> productProducer_ {nullptr};
    // FIXME: This will be a generic principal when meta data is fixed.
    cet::exempt_ptr<EventPrincipal> onDemandPrincipal_ {nullptr};
    mutable RangeSet rangeOfValidity_ {RangeSet::invalid()};
  };  // Group

  inline
  std::ostream&
  operator<<(std::ostream& os, Group const& g)
  {
    g.write(os);
    return os;
  }

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_Group_h */
