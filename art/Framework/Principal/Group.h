#ifndef art_Framework_Principal_Group_h
#define art_Framework_Principal_Group_h
// vim: set sw=2 expandtab :

//
//  A collection of information related to a single EDProduct.
//

#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Common/fwd.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/EDProductGetter.h"
#include "canvas/Persistency/Common/fwd.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Persistency/Provenance/fwd.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/exempt_ptr.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

namespace art {

  class Group final : public EDProductGetter {
  public:
    ~Group();

    enum class grouptype { normal = 0, assns = 1, assnsWithData = 2 };

    Group(DelayedReader*,
          BranchDescription const&,
          std::unique_ptr<RangeSet>&&,
          grouptype const gt,
          std::unique_ptr<EDProduct>&& edp = nullptr);

    // EDProductGetter interface
    EDProduct const* getIt_() const override;
    EDProduct const* anyProduct() const;
    EDProduct const* uniqueProduct() const;
    EDProduct const* uniqueProduct(TypeID const&) const;
    bool resolveProductIfAvailable(TypeID wanted_wrapper = TypeID{}) const;
    bool tryToResolveProduct(TypeID const&);

    // Allows user module to remove a large fetched data product
    // after copying it.
    void removeCachedProduct();

    // Metadata
    BranchDescription const& productDescription() const noexcept;
    ProductID productID() const;
    RangeSet const& rangeOfValidity() const;
    bool productAvailable() const;
    cet::exempt_ptr<ProductProvenance const> productProvenance() const;

    // Setting internal product provenance and product pointers.

    // Called by Principal::ctor_read_provenance()
    // Called by Principal::insert_pp
    //   Called by RootDelayedReader::getProduct_
    void setProductProvenance(std::unique_ptr<ProductProvenance const>&&);

    // Called by Principal::put
    void setProductAndProvenance(std::unique_ptr<ProductProvenance const>&&,
                                 std::unique_ptr<EDProduct>&&,
                                 std::unique_ptr<RangeSet>&&);

  private:
    BranchDescription const& branchDescription_;

    // Back pointer to the delayed reader in the principal that owns
    // us.
    cet::exempt_ptr<DelayedReader const> const delayedReader_;
    // Used to serialize access to productProvenance_, product_,
    // rangeSet_, partnerProduct_, baseProduct_, and
    // partnerBaseProduct_.  This is recursive because sometimes we
    // may need to replace the product provenance when merging run or
    // subRun data products while checking if the product is
    // available, which we may do while resolving a a product with
    // this locked to make the updating of provenance and product
    // pointers together one atomic transaction.
    mutable std::recursive_mutex mutex_{};
    // The product provenance for the data product.
    // Note: Modified by setProductProvenance (called by Principal ctors and
    // Principal::insert_pp (called by Principal::put).
    std::atomic<ProductProvenance const*> productProvenance_{nullptr};
    // The wrapped data product itself.
    // Note: Modified by setProduct (called by Principal::put)
    // Note: Modified by removeCachedProduct.
    // Note: Modified by resolveProductIfAvailable.
    mutable std::atomic<EDProduct*> product_;
    // Note: Modified by setProduct (called by Principal put).
    // Note: Modified by removeCachedProduct.
    // Note: Modified by resolveProductIfAvailable.
    mutable std::atomic<RangeSet*> rangeSet_;
    // Are we normal, assns, or assnsWithData?
    grouptype const grpType_;
    //
    //  AssnsGroup
    //
    // Note: Modified by setProduct (called by Principal put).
    // Note: Modified by removeCachedProduct.
    // Note: Modified by resolveProductIfAvailable.
    mutable std::atomic<EDProduct*> partnerProduct_{nullptr};
    //
    //  AssnsGroupWithData
    //
    // Note: Modified by setProduct.
    // Note: Modified by removeCachedProduct.
    // Note: Modified by resolveProductIfAvailable.
    mutable std::atomic<EDProduct*> baseProduct_{nullptr};
    // Note: Modified by setProduct.
    // Note: Modified by removeCachedProduct.
    // Note: Modified by resolveProductIfAvailable.
    mutable std::atomic<EDProduct*> partnerBaseProduct_{nullptr};
  };

  std::optional<GroupQueryResult> resolve_unique_product(
    std::vector<cet::exempt_ptr<art::Group>> const& groups,
    art::WrappedTypeID const& wrapped);

  std::vector<GroupQueryResult> resolve_products(
    std::vector<cet::exempt_ptr<art::Group>> const& groups,
    art::TypeID const& wrapped_type);

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_Group_h */
