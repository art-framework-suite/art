#include "art/Framework/Principal/Group.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Worker.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "canvas/Utilities/WrappedTypeID.h"
#include "cetlib_except/demangle.h"
#include "range/v3/view.hpp"

#include <iostream>
#include <string>

using namespace std;

namespace art {

  using namespace detail;

  Group::~Group()
  {
    delete productProvenance_.load();
    delete product_.load();
    delete rangeSet_.load();
    delete partnerProduct_.load();
    delete baseProduct_.load();
    delete partnerBaseProduct_.load();
  }

  Group::Group(DelayedReader* reader,
               BranchDescription const& bd,
               unique_ptr<RangeSet>&& rs,
               grouptype const gt,
               unique_ptr<EDProduct>&& edp /*= nullptr*/)
    : branchDescription_{bd}
    , delayedReader_{reader}
    , product_{edp.release()}
    , rangeSet_{rs.release()}
    , grpType_{gt}
  {}

  EDProduct const*
  Group::getIt_() const
  {
    std::lock_guard sentry{mutex_};
    if (grpType_ == grouptype::normal) {
      resolveProductIfAvailable();
      return product_.load();
    }
    return uniqueProduct();
  }

  EDProduct const*
  Group::anyProduct() const
  {
    std::lock_guard sentry{mutex_};
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
    std::lock_guard sentry{mutex_};
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
    std::lock_guard sentry{mutex_};
    if (product_.load() == nullptr) {
      return nullptr;
    }
    if (grpType_ == grouptype::normal) {
      return product_.load();
    }

    auto assns_type_ids = product_.load()->getTypeIDs();
    if (grpType_ == grouptype::assns) {
      assert(assns_type_ids.size() == 2ull);
      if (wanted_wrapper_type ==
          assns_type_ids.at(product_metatype::RightLeft)) {
        return partnerProduct_.load();
      }
      return product_.load();
    }

    assert(assns_type_ids.size() == 4ull);
    if (wanted_wrapper_type == assns_type_ids.at(product_metatype::RightLeft)) {
      return partnerBaseProduct_.load();
    }
    if (wanted_wrapper_type == assns_type_ids.at(product_metatype::LeftRight)) {
      return baseProduct_.load();
    }
    if (wanted_wrapper_type ==
        assns_type_ids.at(product_metatype::RightLeftData)) {
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
    std::lock_guard sentry{mutex_};
    return *rangeSet_.load();
  }

  cet::exempt_ptr<ProductProvenance const>
  Group::productProvenance() const
  {
    std::lock_guard sentry{mutex_};
    return productProvenance_.load();
  }

  // Called by Principal::ctor_read_provenance()
  // Called by Principal::insert_pp
  //   Called by RootDelayedReader::getProduct_
  void
  Group::setProductProvenance(unique_ptr<ProductProvenance const>&& pp)
  {
    std::lock_guard sentry{mutex_};
    delete productProvenance_.load();
    productProvenance_ = pp.release();
  }

  // Called by Principal::put
  void
  Group::setProductAndProvenance(unique_ptr<ProductProvenance const>&& pp,
                                 unique_ptr<EDProduct>&& edp,
                                 unique_ptr<RangeSet>&& rs)
  {
    std::lock_guard sentry{mutex_};
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
    std::lock_guard sentry{mutex_};
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
    std::lock_guard sentry{mutex_};
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
    std::lock_guard sentry{mutex_};
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
      // provenance if run or subRun data product merging creates a
      // new provenance.
      product_ =
        delayedReader_
          ->getProduct(this, branchDescription_.productID(), *rangeSet_.load())
          .release();
      if (product_.load() == nullptr) {
        // We failed to get the master product, hopeless.
        return false;
      }
    }

    if (!wanted_wrapper_type) {
      // The type of the product is not known, therefore the on-disk
      // representation is sufficient.
      return true;
    }

    if (grpType_ == grouptype::normal) {
      // If we get here, we have successfully read a normal product.
      return true;
    }

    assert(grpType_ != grouptype::normal);
    auto normal_metatype = (grpType_ == grouptype::assns) ?
                             product_metatype::LeftRight :
                             product_metatype::LeftRightData;

    auto assns_type_ids = product_.load()->getTypeIDs();
    assert(!assns_type_ids.empty());

    if (wanted_wrapper_type == assns_type_ids.at(normal_metatype)) {
      return true;
    }

    auto partner_metatype = (grpType_ == grouptype::assns) ?
                              product_metatype::RightLeft :
                              product_metatype::RightLeftData;
    if (wanted_wrapper_type == assns_type_ids.at(partner_metatype)) {
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

    assert(grpType_ == grouptype::assnsWithData);

    if (wanted_wrapper_type == assns_type_ids.at(product_metatype::LeftRight)) {
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
    std::lock_guard sentry{mutex_};
    resolveProductIfAvailable(wanted_wrapper);

    // If the product is a dummy filler, it will now be marked unavailable.
    return productAvailable();
  }

  std::optional<GroupQueryResult>
  resolve_unique_product(
    std::vector<cet::exempt_ptr<art::Group>> const& product_groups,
    art::WrappedTypeID const& wrapped)
  {
    auto by_process_name = [](auto const ga, auto const gb) {
      return ga->productDescription().processName() ==
             gb->productDescription().processName();
    };

    // We group product groups according to their process names.  The
    // product groups have already been assembled in reverse-process
    // history.  The first process with a match wins.  Note that it is
    // an error for there to be multiple matches per process.
    for (auto const groups_per_process :
         ranges::views::group_by(product_groups, by_process_name)) {
      // Keep track of all matched groups so that a helpful error
      // message can be reported.
      std::vector<cet::exempt_ptr<art::Group>> matched_groups;
      for (auto const group : groups_per_process) {
        if (group->tryToResolveProduct(wrapped.wrapped_product_type)) {
          matched_groups.emplace_back(group.get());
        }
      }

      if (auto const num_matches = matched_groups.size(); num_matches == 1) {
        return std::make_optional(GroupQueryResult{matched_groups[0]});
      } else if (num_matches > 1) {
        Exception e{errors::ProductNotFound};
        e << "Found " << num_matches
          << " products rather than one that match all criteria\n"
          << "  C++ type: " << wrapped.product_type << "\n";
        for (auto group : matched_groups) {
          e << "  " << group->productDescription().inputTag() << '\n';
        }
        throw e;
      }
    }
    return std::nullopt;
  }

  std::vector<GroupQueryResult>
  resolve_products(std::vector<cet::exempt_ptr<art::Group>> const& groups,
                   art::TypeID const& wrapped_type)
  {
    std::vector<GroupQueryResult> results;
    for (auto group : groups) {
      if (group->tryToResolveProduct(wrapped_type)) {
        results.emplace_back(group.get());
      }
    }
    return results;
  }

} // namespace art
