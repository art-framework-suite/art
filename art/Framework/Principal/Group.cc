#include "art/Framework/Principal/Group.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Worker.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "cetlib_except/demangle.h"
#include "hep_concurrency/RecursiveMutex.h"

#include <iostream>
#include <string>

using namespace std;

namespace art {

  Group::~Group()
  {
    delete productProvenance_.load();
    productProvenance_ = nullptr;
    delete product_.load();
    product_ = nullptr;
    delete rangeSet_.load();
    rangeSet_ = nullptr;
    delete partnerProduct_.load();
    partnerProduct_ = nullptr;
    delete baseProduct_.load();
    baseProduct_ = nullptr;
    delete partnerBaseProduct_.load();
    partnerBaseProduct_ = nullptr;
  }

  // normal
  Group::Group(DelayedReader* reader,
               BranchDescription const& bd,
               unique_ptr<RangeSet>&& rs,
               TypeID const& wrapper_type,
               unique_ptr<EDProduct>&& edp /*= nullptr*/)
    : branchDescription_{bd}
    , delayedReader_{reader}
    , wrapperType_{wrapper_type}
  {
    productProvenance_ = nullptr;
    product_ = edp.release();
    rangeSet_ = rs.release();
    partnerProduct_ = nullptr;
    baseProduct_ = nullptr;
    partnerBaseProduct_ = nullptr;
  }

  // normal, put
  Group::Group(DelayedReader* reader,
               BranchDescription const& bd,
               unique_ptr<RangeSet>&& rs,
               unique_ptr<EDProduct>&& edp,
               TypeID const& wrapper_type)
    : Group{reader, bd, move(rs), wrapper_type, move(edp)}
  {}

  // assns
  Group::Group(DelayedReader* reader,
               BranchDescription const& bd,
               unique_ptr<RangeSet>&& rs,
               TypeID const& primary_wrapper_type,
               TypeID const& partner_wrapper_type,
               unique_ptr<EDProduct>&& edp /*= nullptr*/)
    : branchDescription_{bd}
    , delayedReader_{reader}
    , grpType_{grouptype::assns}
    , wrapperType_{primary_wrapper_type}
    , partnerWrapperType_{partner_wrapper_type}
  {
    productProvenance_ = nullptr;
    product_ = edp.release();
    rangeSet_ = rs.release();
    partnerProduct_ = nullptr;
    baseProduct_ = nullptr;
    partnerBaseProduct_ = nullptr;
  }

  // assns, put
  Group::Group(DelayedReader* reader,
               BranchDescription const& bd,
               unique_ptr<RangeSet>&& rs,
               unique_ptr<EDProduct>&& edp,
               TypeID const& primary_wrapper_type,
               TypeID const& partner_wrapper_type)
    : Group{reader,
            bd,
            move(rs),
            primary_wrapper_type,
            partner_wrapper_type,
            move(edp)}
  {}

  // assnsWithData
  Group::Group(DelayedReader* reader,
               BranchDescription const& bd,
               unique_ptr<RangeSet>&& rs,
               TypeID const& primary_wrapper_type,
               TypeID const& partner_wrapper_type,
               TypeID const& base_wrapper_type,
               TypeID const& partner_base_wrapper_type,
               unique_ptr<EDProduct>&& edp /*= nullptr*/)
    : branchDescription_{bd}
    , delayedReader_{reader}
    , grpType_{grouptype::assnsWithData}
    , wrapperType_{primary_wrapper_type}
    , partnerWrapperType_{partner_wrapper_type}
    , baseWrapperType_{base_wrapper_type}
    , partnerBaseWrapperType_{partner_base_wrapper_type}
  {
    productProvenance_ = nullptr;
    product_ = edp.release();
    rangeSet_ = rs.release();
    partnerProduct_ = nullptr;
    baseProduct_ = nullptr;
    partnerBaseProduct_ = nullptr;
  }

  // assnsWithData, put
  Group::Group(DelayedReader* reader,
               BranchDescription const& bd,
               unique_ptr<RangeSet>&& rs,
               unique_ptr<EDProduct>&& edp,
               TypeID const& primary_wrapper_type,
               TypeID const& partner_wrapper_type,
               TypeID const& base_wrapper_type,
               TypeID const& partner_base_wrapper_type)
    : Group{reader,
            bd,
            move(rs),
            primary_wrapper_type,
            partner_wrapper_type,
            base_wrapper_type,
            partner_base_wrapper_type,
            move(edp)}
  {}

  EDProduct const*
  Group::getIt_() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    if (grpType_ == grouptype::normal) {
      resolveProductIfAvailable();
      return product_.load();
    }
    return uniqueProduct();
  }

  EDProduct const*
  Group::anyProduct() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    if (grpType_ == grouptype::normal) {
      return product_.load();
    }
    EDProduct* result = product_.load();
    if (result == nullptr) {
      result = partnerProduct_.load();
    }
    if (grpType_ == grouptype::assns) {
      return result;
    }
    if (result != nullptr) {
      return result;
    }
    result = baseProduct_.load();
    if (result == nullptr) {
      result = partnerBaseProduct_.load();
    }
    return result;
  }

  EDProduct const*
  Group::uniqueProduct() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    if (grpType_ == grouptype::normal) {
      return product_.load();
    }
    throw Exception(errors::LogicError, "AmbiguousProduct")
      << cet::demangle_symbol(typeid(*this).name())
      << " was asked for a held product (uniqueProduct()) "
      << "without specifying which one was wanted.\n";
  }

  EDProduct const*
  Group::uniqueProduct(TypeID const& wanted_wrapper_type) const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    if (grpType_ == grouptype::normal) {
      return product_.load();
    }
    if (grpType_ == grouptype::assns) {
      if (wanted_wrapper_type == partnerWrapperType_) {
        return partnerProduct_.load();
      }
      return product_.load();
    }
    if (wanted_wrapper_type == partnerBaseWrapperType_) {
      return partnerBaseProduct_.load();
    }
    if (wanted_wrapper_type == baseWrapperType_) {
      return baseProduct_.load();
    }
    if (wanted_wrapper_type == partnerWrapperType_) {
      return partnerProduct_.load();
    }
    return product_.load();
  }

  BranchDescription const&
  Group::productDescription() const
  {
    return branchDescription_;
  }

  ProductID
  Group::productID() const
  {
    return branchDescription_.productID();
  }

  RangeSet const&
  Group::rangeOfValidity() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return *rangeSet_.load();
  }

  cet::exempt_ptr<ProductProvenance const>
  Group::productProvenance() const
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    return productProvenance_.load();
  }

  // Called by Principal::ctor_read_provenance()
  // Called by Principal::insert_pp
  //   Called by RootDelayedReader::getProduct_
  void
  Group::setProductProvenance(unique_ptr<ProductProvenance const>&& pp)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    delete productProvenance_.load();
    productProvenance_ = pp.release();
  }

  // Called by Principal::put
  void
  Group::setProductAndProvenance(unique_ptr<ProductProvenance const>&& pp,
                                 unique_ptr<EDProduct>&& edp,
                                 unique_ptr<RangeSet>&& rs)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    delete productProvenance_.load();
    productProvenance_ = pp.release();
    delete product_.load();
    product_ = edp.release();
    delete rangeSet_.load();
    rangeSet_ = rs.release();
  }

  void
  Group::removeCachedProduct()
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    if (branchDescription_.produced()) {
      throw Exception(errors::LogicError, "Group::removeCachedProduct():")
        << "Attempt to remove a produced product!\n"
        << "This routine should only be used to remove large data products "
        << "read from disk (like raw digits).\n";
    }
    delete product_.load();
    product_ = nullptr;
    if (grpType_ == grouptype::normal) {
      return;
    }
    delete partnerProduct_.load();
    partnerProduct_ = nullptr;
    if (grpType_ == grouptype::assns) {
      return;
    }
    delete baseProduct_.load();
    baseProduct_ = nullptr;
    delete partnerBaseProduct_.load();
    partnerBaseProduct_ = nullptr;
    delete rangeSet_.load();
    rangeSet_ = new RangeSet{};
  }

  bool
  Group::productAvailable() const
  {
    if (branchDescription_.dropped()) {
      // Not a product we are producing this time around, and it is not
      // present in any of the input files we have opened so far.
      return false;
    }
    assert(branchDescription_.present() || branchDescription_.produced());
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    bool availableAfterCombine{false};
    if ((branchDescription_.branchType() == InSubRun) ||
        (branchDescription_.branchType() == InRun)) {
      availableAfterCombine =
        delayedReader_->isAvailableAfterCombine(branchDescription_.productID());
    }
    auto status = productstatus::uninitialized();
    if (productProvenance_.load() == nullptr) {
      // No provenance, must be a produced product which has not been
      // put yet, or a non-produced product that is available after
      // combine (agggregation) and not yet read, or a non-produced
      // product in a secondary file that has not yet been opened.
      if (!branchDescription_.produced()) {
        if (availableAfterCombine) {
          // No provenance, not produced, but we can get it from the
          // input, we just have not done so yet.  Claim the product is
          // available.
          return true;
        }
        // Not produced, not availableAfterCombine, must be in a
        // secondary file that has not yet been opened. We report it as
        // not available so that the Principal getBy* routines will try
        // the next secondary file.
        return false;
      }
      if (product_.load()) {
        throw Exception(errors::LogicError, "Group::status():")
          << "We have a produced product, the product has been put(), but "
             "there is no provenance!\n";
      }
      // We have a product product which has not been put(), and has no
      // provenance (as it should be).
      status = productstatus::neverCreated();
    } else {
      // Not a produced product, and not yet delay read, use the status
      // from the on-file provenance.
      status = productProvenance_.load()->productStatus();
    }
    if ((branchDescription_.branchType() == InSubRun) ||
        (branchDescription_.branchType() == InRun)) {
      if (!availableAfterCombine) {
        // We know this is a produced run or subrun product which is not
        // present in any fragments.
        return status == productstatus::present();
      }
    }
    // We now know we are either an event or results product, or we are
    // a run or subrun product that can become valid through product
    // combination (aggregation).
    if (status == productstatus::dummyToPreventDoubleCount()) {
      // We now know that we are a run or subrun product that can become
      // valid through product combination (aggregation).  Special case
      // this and report the product as available even though the the
      // provenance product status is a special flag that is not the
      // present status.
      // This is here to allow fetching of a product specially marked by
      // RootOutputFile as a dummy with an invalid range set created to
      // prevent double-counting when combining run/subrun products.  We
      // allow the fetch to happen because the call to
      // isPossiblyAvailable above determined that the fetch will result
      // in a valid and present product, even though this particular
      // dummy one is not.
      return true;
    }
    // Note: Technically this is not necessary since the Wrapper present
    //       flag covers this case, but this way we never do the I/O on
    //       the product if we already have the provenance.
    return status == productstatus::present();
  }

  bool
  Group::resolveProductIfAvailable(
    TypeID wanted_wrapper_type /*= TypeID{}*/) const
  {
    // Validate wanted_wrapper_type.
    auto throwResolveLogicError = [this, &wanted_wrapper_type] {
      throw Exception(errors::LogicError, "INTERNAL ERROR: ")
        << cet::demangle_symbol(typeid(*this).name())
        << " cannot resolve wanted product of type "
        << wanted_wrapper_type.className() << ".\n";
    };
    if (!wanted_wrapper_type) {
      wanted_wrapper_type = wrapperType_;
    }
    if (grpType_ == grouptype::normal) {
      if (wanted_wrapper_type != wrapperType_) {
        throwResolveLogicError();
      }
    } else if (grpType_ == grouptype::assns) {
      if ((wanted_wrapper_type != wrapperType_) &&
          (wanted_wrapper_type != partnerWrapperType_)) {
        throwResolveLogicError();
      }
    } else {
      if ((wanted_wrapper_type != wrapperType_) &&
          (wanted_wrapper_type != partnerWrapperType_) &&
          (wanted_wrapper_type != baseWrapperType_) &&
          (wanted_wrapper_type != partnerBaseWrapperType_)) {
        throwResolveLogicError();
      }
    }
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    // Now try to get the master product.
    if (product_.load() == nullptr) {
      // Not already resolved.
      if (branchDescription_.produced()) {
        // Never produced, hopeless.
        return false;
      }
      if (!productAvailable()) {
        // Not possible to get it, hopeless.
        return false;
      }
      // Now try to read it.
      // Note: This may call back to us to update the product
      // provenance if run or subRun data product merging
      // creates a new provenance.
      product_ = delayedReader_
                   ->getProduct(this,
                                branchDescription_.productID(),
                                wrapperType_,
                                *rangeSet_.load())
                   .release();
      if (product_.load() == nullptr) {
        // We failed to get the master product, hopeless.
        return false;
      }
    }
    if (wanted_wrapper_type == wrapperType_) {
      // They wanted the master product and we just got it, all done.
      return true;
    }
    if (wanted_wrapper_type == partnerWrapperType_) {
      if (partnerProduct_.load() != nullptr) {
        // They wanted the partner product, and we have already made it, done.
        return true;
      }
      // They want the partner product, ask the wrapper to make it for us,
      // who ends up asking the assns to do it.
      partnerProduct_ =
        product_.load()->makePartner(wanted_wrapper_type.typeInfo()).release();
      return partnerProduct_.load() != nullptr;
    }
    if (wanted_wrapper_type == baseWrapperType_) {
      if (baseProduct_.load() != nullptr) {
        // They wanted the base product, and we have already made it, done.
        return true;
      }
      // They want the base, ask the wrapper to make it for us,
      // who ends up asking the assns to do it.
      baseProduct_ =
        product_.load()->makePartner(wanted_wrapper_type.typeInfo()).release();
      return baseProduct_.load() != nullptr;
    }
    if (partnerBaseProduct_.load() != nullptr) {
      // They wanted the partner base product, and we have already made it,
      // done.
      return true;
    }
    partnerBaseProduct_ =
      product_.load()->makePartner(wanted_wrapper_type.typeInfo()).release();
    return partnerBaseProduct_.load() != nullptr;
  }

  bool
  Group::tryToResolveProduct(TypeID const& wanted_wrapper)
  {
    hep::concurrency::RecursiveMutexSentry sentry{mutex_, __func__};
    if (wanted_wrapper) {
      resolveProductIfAvailable(wanted_wrapper);
    } else {
      resolveProductIfAvailable();
    }
    // If the product is a dummy filler, it will now be marked unavailable.
    if (!productAvailable()) {
      return false;
    }
    return true;
  }

} // namespace art
