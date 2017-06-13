#ifndef art_Framework_Principal_Group_h
#define art_Framework_Principal_Group_h
// vim: set sw=2:

//
//  A collection of information related to a single EDProduct.
//

#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/EDProductGetter.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchMapper.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Utilities/Exception.h"
#include "art/Utilities/fwd.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <type_traits>

namespace art {

  //
  // Noncopyable through inheritance from EDProductGetter.
  //

  class Group : public EDProductGetter {

    // The operative part of the GroupFactory system.
    template <typename ... ARGS>
    friend
    std::unique_ptr<Group>
    gfactory::detail::
    make_group(BranchDescription const&, ARGS&& ... args);

  public:

    Group() = default;

  protected:

    //
    // Use GroupFactory to make.
    //
    Group(BranchDescription const& bd,
          ProductID const& pid,
          RangeSet&& rs,
          art::TypeID const& wrapper_type,
          std::unique_ptr<EDProduct>&& edp = nullptr,
          cet::exempt_ptr<Worker> productProducer = cet::exempt_ptr<Worker> {})
      : wrapperType_{wrapper_type}
      , product_{std::move(edp)}
      , branchDescription_{&bd}
      , pid_{pid}
      , productProducer_{productProducer}
      , rangeOfValidity_{std::move(rs)}
      {}

    Group(BranchDescription const& bd,
          ProductID const& pid,
          RangeSet&& rs,
          cet::exempt_ptr<Worker> productProducer,
          art::TypeID const& wrapper_type)
      : Group{bd, pid, std::move(rs), wrapper_type, nullptr, productProducer}
      {}

    Group(BranchDescription const& bd,
          ProductID const& pid,
          RangeSet&& rs,
          std::unique_ptr<EDProduct>&& edp,
          art::TypeID const& wrapper_type)
      : Group{bd, pid, std::move(rs), wrapper_type, std::move(edp)}
      {}

  public:

    // product is not available (dropped or never created)
    bool productUnavailable() const;

    bool isReady() const override
    {
      return true;
    }

    EDProduct const* getIt() const override
    {
      resolveProductIfAvailable(producedWrapperType());
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

    bool resolveProduct(TypeID const&) const override;

    bool resolveProductIfAvailable(TypeID const&) const override;

    void write(std::ostream& os) const;

    ProductID const& productID() const
    {
      return pid_;
    };

    TypeID const& producedWrapperType() const
    {
      return wrapperType_;
    }

    virtual void removeCachedProduct() const;

    RangeSet const& rangeOfValidity() const { return rangeOfValidity_; }

  protected:

    std::unique_ptr<EDProduct> obtainDesiredProduct(TypeID const&) const;

    void setProduct(std::unique_ptr<EDProduct>&& prod) const;

    [[noreturn]] void throwResolveLogicError (TypeID const& wanted_wrapper_type) const;

  private:

    bool dropped() const;

  private:

    TypeID wrapperType_ {};
    cet::exempt_ptr<BranchMapper const> ppResolver_ {nullptr};
    cet::exempt_ptr<DelayedReader const> productResolver_ {nullptr};
    mutable std::unique_ptr<EDProduct> product_ {nullptr};
    cet::exempt_ptr<BranchDescription const> branchDescription_ {nullptr};
    mutable ProductID pid_ {};
    cet::exempt_ptr<Worker> productProducer_ {nullptr};
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
