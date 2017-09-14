#ifndef art_Framework_Principal_Group_h
#define art_Framework_Principal_Group_h
// vim: set sw=2 expandtab :

//
//  A collection of information related to a single EDProduct.
//

#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/DelayedReader.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/EDProductGetter.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Utilities/Exception.h"
#include "art/Utilities/fwd.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"

#include <algorithm>
#include <atomic>
#include <iostream>
#include <memory>
#include <mutex>
#include <type_traits>

namespace art {

class Principal;

class Group final : public EDProductGetter {

private: // TYPES

  enum class grouptype {
      normal // 0
    , assns // 1
    , assnsWithData // 2
  };

public: // MEMBER FUNCTIONS -- Special Member Functions

  virtual
  ~Group();

  Group();

  // normal
  Group(Principal*, DelayedReader*, BranchDescription const&, std::unique_ptr<RangeSet>&&, TypeID const& wrapper_type,
        std::unique_ptr<EDProduct>&& edp = nullptr);

  // normal, put
  Group(Principal*, DelayedReader*, BranchDescription const&, std::unique_ptr<RangeSet>&&, std::unique_ptr<EDProduct>&&,
        art::TypeID const& wrapper_type);

  // assns
  Group(Principal*, DelayedReader*, BranchDescription const&, std::unique_ptr<RangeSet>&&, TypeID const& primary_wrapper_type,
        TypeID const& partner_wrapper_type, std::unique_ptr<EDProduct>&& edp = nullptr);

  // assns, put
  Group(Principal*, DelayedReader*, BranchDescription const&, std::unique_ptr<RangeSet>&&, std::unique_ptr<EDProduct>&&,
        TypeID const& primary_wrapper_type, TypeID const& partner_wrapper_type);

  // assnsWithData
  Group(Principal*, DelayedReader*, BranchDescription const&, std::unique_ptr<RangeSet>&&, TypeID const& primary_wrapper_type,
        TypeID const& partner_wrapper_type, TypeID const& base_wrapper_type, TypeID const& partner_base_wrapper_type,
        std::unique_ptr<EDProduct>&& edp = nullptr);

  // assnsWithData, put
  Group(Principal*, DelayedReader*, BranchDescription const&, std::unique_ptr<RangeSet>&&, std::unique_ptr<EDProduct>&&,
        TypeID const& primary_wrapper_type, TypeID const& partner_wrapper_type, TypeID const& base_wrapper_type,
        TypeID const& partner_base_wrapper_type);

  Group(Group const&) = delete;

  Group(Group&&) = delete;

  Group&
  operator=(Group const&) = delete;

  Group&
  operator=(Group&&) = delete;

public: // MEMBER FUNCTIONS -- EDProductGetter interface

  virtual
  EDProduct const*
  getIt_() const override;

  EDProduct const*
  anyProduct() const;

  EDProduct const*
  uniqueProduct() const;

  EDProduct const*
  uniqueProduct(TypeID const&) const;

  bool
  resolveProductIfAvailable(TypeID wanted_wrapper = TypeID{}) const;

  bool
  tryToResolveProduct(TypeID const&);

public: // MEMBER FUNCTIONS -- Memory-saving API

  // Allows user module to remove a large fetched data product
  // after copying it.
  void
  removeCachedProduct();

public: // MEMBER FUNCTIONS -- Metadata API

  BranchDescription const&
  productDescription() const;

  ProductID
  productID() const;

  RangeSet const&
  rangeOfValidity() const;

  bool
  productAvailable() const;

  cet::exempt_ptr<ProductProvenance const>
  productProvenance() const;

public: // MEMBER FUNCTIONS -- API for setting internal product provenance and product pointers.

  // Called by Principal::ctor_read_provenance()
  // Called by Principal::insert_pp
  //   Called by RootDelayedReader::getProduct_
  void
  setProductProvenance(std::unique_ptr<ProductProvenance const>&&);

  // Called by Principal::put
  void
  setProductAndProvenance(std::unique_ptr<ProductProvenance const>&&, std::unique_ptr<EDProduct>&&, std::unique_ptr<RangeSet>&&);

private:

  cet::exempt_ptr<BranchDescription const> const
  branchDescription_{};

  // Back pointer to the principal that owns us.
  Principal* const
  principal_{nullptr};

  // Back pointer to the delayed reader in the principal that owns us.
  cet::exempt_ptr<DelayedReader const> const
  delayedReader_{};

  // Used to serialize access to productProvenance_, product_, rangeSet_,
  // partnerProduct_, baseProduct_, and partnerBaseProduct_.
  // Note: threading: This is recursive because sometimes we may need to
  // Note: threading: replace the product provenance when merging run or
  // Note: threading: subRun data products while checking if the product
  // Note: threading: is available, which we may do while resolving a
  // Note: threading: a product with this mutex locked to make the updating
  // Note: threading: of provenance and product pointers atomic.
  mutable
  std::recursive_mutex
  mutex_{};

  // The product provenance for the data product.
  // Note: Modified by setProductProvenance (called by Principal ctors and Principal::insert_pp (called by Principal::put).
  std::atomic<ProductProvenance const*>
  productProvenance_{nullptr};

  // The wrapped data product itself.
  // Note: Modified by setProduct (called by Principal::put)
  // Note: Modified by removeCachedProduct.
  // Note: Modified by resolveProductIfAvailable.
  mutable
  std::atomic<EDProduct*>
  product_{nullptr};

  // Note: Modified by setProduct (called by Principal put).
  // Note: Modified by removeCachedProduct.
  // Note: Modified by resolveProductIfAvailable.
  mutable
  std::atomic<RangeSet*>
  rangeSet_{new RangeSet{}};

  // Are we normal, assns, or assnsWithData?
  grouptype const
  grpType_{grouptype::normal};

  //
  //  Normal Group
  //

  TypeID const
  wrapperType_{};

  //
  //  AssnsGroup
  //

  TypeID const
  partnerWrapperType_{};

  // Note: Modified by setProduct (called by Principal put).
  // Note: Modified by removeCachedProduct.
  // Note: Modified by resolveProductIfAvailable.
  mutable
  std::atomic<EDProduct*>
  partnerProduct_{nullptr};

  //
  //  AssnsGroupWithData
  //

  TypeID const
  baseWrapperType_{};

  TypeID const
  partnerBaseWrapperType_{};

  // Note: Modified by setProduct.
  // Note: Modified by removeCachedProduct.
  // Note: Modified by resolveProductIfAvailable.
  mutable
  std::atomic<EDProduct*>
  baseProduct_{nullptr};

  // Note: Modified by setProduct.
  // Note: Modified by removeCachedProduct.
  // Note: Modified by resolveProductIfAvailable.
  mutable
  std::atomic<EDProduct*>
  partnerBaseProduct_{nullptr};

};

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_Group_h */
