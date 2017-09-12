#include "art/Framework/Principal/Group.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Worker.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "cetlib_except/demangle.h"

#include <iostream>
#include <string>

using namespace std;

namespace art {

Group::
~Group()
{
  delete productProvenance_;
  productProvenance_ = nullptr;
  delete product_;
  product_ = nullptr;
  delete rangeSet_;
  rangeSet_ = nullptr;
  delete partnerProduct_;
  partnerProduct_ = nullptr;
  delete baseProduct_;
  baseProduct_ = nullptr;
  delete partnerBaseProduct_;
  partnerBaseProduct_ = nullptr;
}

Group::
Group()
  : EDProductGetter()
  , branchDescription_{}
  , principal_{nullptr}
  , delayedReader_{}
  , mutex_{}
  , productProvenance_{nullptr}
  , product_{nullptr}
  , rangeSet_{new RangeSet{}}
  , grpType_{grouptype::normal}
  , wrapperType_{}
  , partnerWrapperType_{}
  , partnerProduct_{nullptr}
  , baseWrapperType_{}
  , partnerBaseWrapperType_{}
  , baseProduct_{nullptr}
  , partnerBaseProduct_{nullptr}
{
}

// normal
Group::
Group(Principal* principal, DelayedReader* reader, BranchDescription const& bd, unique_ptr<RangeSet>&& rs,
      TypeID const& wrapper_type, unique_ptr<EDProduct>&& edp /*= nullptr*/)
  : EDProductGetter()
  , branchDescription_{&bd}
  , principal_{principal}
  , delayedReader_{reader}
  , mutex_{}
  , productProvenance_{nullptr}
  , product_{edp.release()}
  , rangeSet_{rs.release()}
  , grpType_{grouptype::normal}
  , wrapperType_{wrapper_type}
  , partnerWrapperType_{}
  , partnerProduct_{nullptr}
  , baseWrapperType_{}
  , partnerBaseWrapperType_{}
  , baseProduct_{nullptr}
  , partnerBaseProduct_{nullptr}
{
}

// normal, put
Group::
Group(Principal* principal, DelayedReader* reader, BranchDescription const& bd, unique_ptr<RangeSet>&& rs,
      unique_ptr<EDProduct>&& edp, TypeID const& wrapper_type)
  : Group{principal, reader, bd, move(rs), wrapper_type, move(edp)}
{
}

// assns
Group::
Group(Principal* principal, DelayedReader* reader, BranchDescription const& bd, unique_ptr<RangeSet>&& rs,
      TypeID const& primary_wrapper_type, TypeID const& partner_wrapper_type, unique_ptr<EDProduct>&& edp /*= nullptr*/)
  : EDProductGetter()
  , branchDescription_{&bd}
  , principal_{principal}
  , delayedReader_{reader}
  , mutex_{}
  , productProvenance_{nullptr}
  , product_{edp.release()}
  , rangeSet_{rs.release()}
  , grpType_{grouptype::assns}
  , wrapperType_{primary_wrapper_type}
  , partnerWrapperType_{partner_wrapper_type}
  , partnerProduct_{nullptr}
  , baseWrapperType_{}
  , partnerBaseWrapperType_{}
  , baseProduct_{nullptr}
  , partnerBaseProduct_{nullptr}
{
}

// assns, put
Group::
Group(Principal* principal, DelayedReader* reader, BranchDescription const& bd, unique_ptr<RangeSet>&& rs,
      unique_ptr<EDProduct>&& edp, TypeID const& primary_wrapper_type, TypeID const& partner_wrapper_type)
  : Group{principal, reader, bd, move(rs), primary_wrapper_type, partner_wrapper_type, move(edp)}
{
}

// assnsWithData
Group::
Group(Principal* principal, DelayedReader* reader, BranchDescription const& bd, unique_ptr<RangeSet>&& rs,
      TypeID const& primary_wrapper_type, TypeID const& partner_wrapper_type, TypeID const& base_wrapper_type,
      TypeID const& partner_base_wrapper_type, unique_ptr<EDProduct>&& edp /*= nullptr*/)
  : EDProductGetter()
  , branchDescription_{&bd}
  , principal_{principal}
  , delayedReader_{reader}
  , mutex_{}
  , productProvenance_{nullptr}
  , product_{edp.release()}
  , rangeSet_{rs.release()}
  , grpType_{grouptype::assnsWithData}
  , wrapperType_{primary_wrapper_type}
  , partnerWrapperType_{partner_wrapper_type}
  , partnerProduct_{nullptr}
  , baseWrapperType_{base_wrapper_type}
  , partnerBaseWrapperType_{partner_base_wrapper_type}
  , baseProduct_{nullptr}
  , partnerBaseProduct_{nullptr}
{
}

// assnsWithData, put
Group::
Group(Principal* principal, DelayedReader* reader, BranchDescription const& bd, unique_ptr<RangeSet>&& rs,
      unique_ptr<EDProduct>&& edp, TypeID const& primary_wrapper_type, TypeID const& partner_wrapper_type,
      TypeID const& base_wrapper_type, TypeID const& partner_base_wrapper_type)
  : Group(principal, reader, bd, move(rs), primary_wrapper_type, partner_wrapper_type, base_wrapper_type, partner_base_wrapper_type,
          move(edp))
{
}

EDProduct const*
Group::
getIt_() const
{
  lock_guard<recursive_mutex> lock_holder{mutex_};
  if (grpType_ == grouptype::normal) {
    resolveProductIfAvailable();
    return product_;
  }
  return uniqueProduct();
}

EDProduct const*
Group::
anyProduct() const
{
  lock_guard<recursive_mutex> lock_holder{mutex_};
  if (grpType_ == grouptype::normal) {
    return product_;
  }
  EDProduct* result = product_;
  if (result == nullptr) {
    result = partnerProduct_;
  }
  if (grpType_ == grouptype::assns) {
    return result;
  }
  if (result != nullptr) {
    return result;
  }
  result = baseProduct_;
  if (result == nullptr) {
    result = partnerBaseProduct_;
  }
  return result;
}

EDProduct const*
Group::
uniqueProduct() const
{
  lock_guard<recursive_mutex> lock_holder{mutex_};
  if (grpType_ == grouptype::normal) {
    return product_;
  }
  throw Exception(errors::LogicError, "AmbiguousProduct")
      << cet::demangle_symbol(typeid(*this).name())
      << " was asked for a held product (uniqueProduct()) "
      << "without specifying which one was wanted.\n";
}

EDProduct const*
Group::
uniqueProduct(TypeID const& wanted_wrapper_type) const
{
  lock_guard<recursive_mutex> lock_holder{mutex_};
  if (grpType_ == grouptype::normal) {
    return product_;
  }
  if (grpType_ == grouptype::assns) {
    if (wanted_wrapper_type == partnerWrapperType_) {
      return partnerProduct_;
    }
    return product_;
  }
  if (wanted_wrapper_type == partnerBaseWrapperType_) {
    return partnerBaseProduct_;
  }
  if (wanted_wrapper_type == baseWrapperType_) {
    return baseProduct_;
  }
  if (wanted_wrapper_type == partnerWrapperType_) {
    return partnerProduct_;
  }
  return product_;
}

BranchDescription const&
Group::
productDescription() const
{
  return *branchDescription_;
}

ProductID
Group::
productID() const
{
  return branchDescription_->productID();
}

RangeSet const&
Group::
rangeOfValidity() const
{
  lock_guard<recursive_mutex> lock_holder{mutex_};
  return *rangeSet_;
}

cet::exempt_ptr<const art::ProductProvenance>
Group::
productProvenance() const
{
  lock_guard<recursive_mutex> lock_holder{mutex_};
  return productProvenance_.load();
}

// Called by Principal::ctor_read_provenance()
// Called by Principal::insert_pp
//   Called by RootDelayedReader::getProduct_
void
Group::
setProductProvenance(unique_ptr<ProductProvenance const>&& pp)
{
  lock_guard<recursive_mutex> lock_holder{mutex_};
  delete productProvenance_;
  productProvenance_ = pp.release();
}

// Called by Principal::put
void
Group::
setProductAndProvenance(unique_ptr<ProductProvenance const>&& pp, unique_ptr<EDProduct>&& edp, unique_ptr<RangeSet>&& rs)
{
  lock_guard<recursive_mutex> lock_holder{mutex_};
  delete productProvenance_;
  productProvenance_ = pp.release();
  delete product_;
  product_ = edp.release();
  delete rangeSet_;
  rangeSet_ = rs.release();
}

void
Group::
removeCachedProduct()
{
  lock_guard<recursive_mutex> lock_holder{mutex_};
  if (branchDescription_->produced()) {
    throw Exception(errors::LogicError, "Group::removeCachedProduct():")
        << "Attempt to remove a produced product!\n"
        << "This routine should only be used to remove large data products "
        << "read from disk (like raw digits).\n";
  }
  delete product_;
  product_ = nullptr;
  if (grpType_ == grouptype::normal) {
    return;
  }
  delete partnerProduct_;
  partnerProduct_ = nullptr;
  if (grpType_ == grouptype::assns) {
    return;
  }
  delete baseProduct_;
  baseProduct_ = nullptr;
  delete partnerBaseProduct_;
  partnerBaseProduct_ = nullptr;
  delete rangeSet_;
  rangeSet_ = new RangeSet{};
}

bool
Group::
productAvailable() const
{
  if (!branchDescription_) {
    // We do not have a branch description at all.
    throw Exception(errors::LogicError, "Group::productAvailable():")
      << "branchDescription_ is the nullptr!\n";
  }

  lock_guard<recursive_mutex> lock_holder{mutex_};
  if (branchDescription_->dropped()) {
    // Not a product we are producing this time around, and it is not
    // present in any of the input files we have opened so far.
    return false;
  }
  bool availableAfterCombine = false;
  if ((branchDescription_->branchType() == InSubRun) || (branchDescription_->branchType() == InRun)) {
    availableAfterCombine = delayedReader_->isAvailableAfterCombine(branchDescription_->productID());
    if (!availableAfterCombine) {
      // This is a run or subrun product and none of the available
      // fragments has a product that is present.  If this is not a
      // produced product, then we are now sure that it is not
      // available. Otherwise either the product has not been put yet,
      // in which case it is unavailable, or we must check the status
      // in the provenance of the put product; we check these two
      // cases later.
      if (!branchDescription_->produced()) {
        // Not produced and not in any available fragments, not available.
        return false;
      }
    }
  }
  auto status = productstatus::uninitialized();
  if (productProvenance_ == nullptr) {
    // No provenance, must be a produced product which has not been
    // put yet, or a non-produced product that is available after
    // combine (agggregation) and not yet read, or a non-produced
    // product in a secondary file that has not yet been opened.
    if (!branchDescription_->produced()) {
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
    if (product_) {
      throw Exception(errors::LogicError, "Group::status():")
          << "We have a produced product, the product has been put(), but there is no provenance!\n";
    }
    // We have a product product which has not been put(), and has no
    // provenance (as it should be).
    status = productstatus::neverCreated();
  }
  else {
    // Not a produced product, and not yet delay read, use the status from the on-file provenance.
    status = productProvenance_.load()->productStatus();
  }
  if ((branchDescription_->branchType() == InSubRun) || (branchDescription_->branchType() == InRun)) {
    if (!availableAfterCombine) {
      // We know this is a produced run or subrun product which is not present in any fragments.
      return status == productstatus::present();
    }
  }
  // We now know we are either an event or results product, or we are a
  // run or subrun product that can become valid through product
  // combination (aggregation).
  if (status == productstatus::dummyToPreventDoubleCount()) {
    // We now know that we are a run or subrun product that can become
    // valid through product combination (aggregation).  Special case
    // this and report the product as available even though the the
    // provenance product status is a special flag that is not the
    // present status.
    // This is here to allow fetching of a product specially marked by
    // RootOutputFile as a dummy with an invalid range set created to
    // prevent double-counting when combining run/subrun products.
    // We allow the fetch to happen because the call to isPossiblyAvailable
    // above determined that the fetch will result in a valid and present
    // product, even though this particular dummy one is not.
    return true;
  }
  // Note: Technically this is not necessary since the Wrapper present flag
  //       covers this case, but this way we never do the I/O on the product
  //       if we already have the provenance.
  return status == productstatus::present();
}

bool
Group::
resolveProductIfAvailable(TypeID wanted_wrapper_type /*= TypeID{}*/) const
{
  // Validate wanted_wrapper_type.
  auto throwResolveLogicError = [this, &wanted_wrapper_type]() {
    throw Exception(errors::LogicError, "INTERNAL ERROR: ")
      << cet::demangle_symbol(typeid(*this).name())
      << " cannot resolve wanted product of type "
      << wanted_wrapper_type.className()
      << ".\n";
  };
  if (!wanted_wrapper_type) {
    wanted_wrapper_type = wrapperType_;
  }
  if (grpType_ == grouptype::normal) {
    if (wanted_wrapper_type != wrapperType_) {
      throwResolveLogicError();
    }
  }
  else if (grpType_ == grouptype::assns) {
    if ((wanted_wrapper_type != wrapperType_) && (wanted_wrapper_type != partnerWrapperType_)) {
      throwResolveLogicError();
    }
  }
  else {
    if ((wanted_wrapper_type != wrapperType_) && (wanted_wrapper_type != partnerWrapperType_) &&
        (wanted_wrapper_type != baseWrapperType_) && (wanted_wrapper_type != partnerBaseWrapperType_)) {
      throwResolveLogicError();
    }
  }
  lock_guard<recursive_mutex> lock_holder{mutex_};
  // Now try to get the master product.
  if (product_ == nullptr) {
    // Not already resolved.
    if (branchDescription_->produced()) {
      // Never produced, hopeless.
      return false;
    }
    if (!productAvailable()) {
      // Not possible to get it, hopeless.
      return false;
    }
    // Now try to read it.
    // Note: threading: This may call back to us to update the product provenance if run or subRun
    // Note: threading: data product merging creates a new provenance.
    product_ = delayedReader_->getProduct(BranchKey{*branchDescription_}, wrapperType_, *rangeSet_).release();
    if (product_ == nullptr) {
      // We failed to get the master product, hopeless.
      return false;
    }
  }
  if (wanted_wrapper_type == wrapperType_) {
    // They wanted the master product and we just got it, all done.
    return true;
  }
  if (wanted_wrapper_type == partnerWrapperType_) {
    if (partnerProduct_ != nullptr) {
      // They wanted the partner product, and we have already made it, done.
      return true;
    }
    // They want the partner product, ask the wrapper to make it for us,
    // who ends up asking the assns to do it.
    partnerProduct_ = product_.load()->makePartner(wanted_wrapper_type.typeInfo()).release();
    return partnerProduct_ != nullptr;
  }
  if (wanted_wrapper_type == baseWrapperType_) {
    if (baseProduct_ != nullptr) {
      // They wanted the base product, and we have already made it, done.
      return true;
    }
    // They want the base, ask the wrapper to make it for us,
    // who ends up asking the assns to do it.
    baseProduct_ = product_.load()->makePartner(wanted_wrapper_type.typeInfo()).release();
    return baseProduct_ != nullptr;
  }
  if (partnerBaseProduct_ != nullptr) {
    // They wanted the partner base product, and we have already made it, done.
    return true;
  }
  partnerBaseProduct_ = product_.load()->makePartner(wanted_wrapper_type.typeInfo()).release();
  return partnerBaseProduct_ != nullptr;
}

bool
Group::
tryToResolveProduct(TypeID const& wanted_wrapper)
{
  lock_guard<recursive_mutex> lock_holder{mutex_};
  if (wanted_wrapper) {
    resolveProductIfAvailable(wanted_wrapper);
  }
  else {
    resolveProductIfAvailable();
  }
  // If the product is a dummy filler, it will now be marked unavailable.
  if (!productAvailable()) {
    return false;
  }
  return true;
}

} // namespace art
