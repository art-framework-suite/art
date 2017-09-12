#include "art/Framework/IO/Root/RootDelayedReader.h"
// vim: sw=2:

#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/IO/Root/RootInputFile.h"
//#include "art/Framework/IO/Root/RootInputTree.h"
#include "art/Framework/IO/Root/detail/resolveRangeSet.h"
#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Common/RefCoreStreamer.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/ProductIDStreamer.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/crc32.h"

#include "TBranch.h"
#include "TBranchElement.h"
#include "TClass.h"

#include <atomic>
#include <cassert>
#include <cstring>
#include <mutex>
#include <utility>
#include <vector>

using namespace std;

namespace art {

RootDelayedReader::
~RootDelayedReader()
{
}

RootDelayedReader::
RootDelayedReader(FileFormatVersion const version,
                  sqlite3* db,
                  vector<input::EntryNumber> const& entrySet,
                  cet::exempt_ptr<input::BranchMap const> branches,
                  TBranch* provenanceBranch,
                  //cet::exempt_ptr<RootInputTree> /*tree*/,
                  int64_t const saveMemoryObjectThreshold,
                  cet::exempt_ptr<RootInputFile> primaryFile,
                  cet::exempt_ptr<BranchIDLists const> bidLists,
                  BranchType const branchType,
                  EventID const eID)
  : DelayedReader()
  , fileFormatVersion_{version}
  , db_{db}
  , entrySet_{entrySet}
  , branches_{branches}
  , provenanceBranch_{provenanceBranch}
  //, tree_{tree}
  , saveMemoryObjectThreshold_{saveMemoryObjectThreshold}
  , primaryFile_{primaryFile}
  , branchIDLists_{bidLists}
  , branchType_{branchType}
  , eventID_{eID}
{
}

void
RootDelayedReader::
setPrincipal_(cet::exempt_ptr<Principal> principal)
{
  principal_ = principal;
}

std::vector<ProductProvenance>
RootDelayedReader::
readProvenance_() const
{
  std::lock_guard<std::recursive_mutex> lock_input(*SharedResourcesRegistry::instance()->getMutexForSource());
  vector<ProductProvenance> ppv;
  auto p_ppv = &ppv;
  provenanceBranch_->SetAddress(&p_ppv);
  // Note: This provenance may be replaced later for
  //       run and subrun products by the combination
  //       process (agggregation).
  input::getEntry(provenanceBranch_, entrySet_[0]);
  return ppv;
}

// FIXME: The following function may need to go somewhere outside of RootDelayedReader.
// For run and subrun products check to see if attempting
// to read them will result in a combination with a valid
// range set.  If not report to the caller that they can
// skip doing the read.
bool
RootDelayedReader::
isAvailableAfterCombine_(ProductID bid) const
{
  if ((branchType_ != InSubRun) && (branchType_ != InRun)) {
    // We only handle run and subrun products here, tell
    // the caller he should proceed.
    return true;
  }
  // Products from files that did not support range sets are
  // worth attempting to read, tell the caller to proceed.
  if (fileFormatVersion_.value_ < 9) {
    return true;
  }
  {
    lock_guard<recursive_mutex> lock_input(*art::SharedResourcesRegistry::instance()->getMutexForSource());
    for (auto I = entrySet_.cbegin(), E = entrySet_.cend(); I != E; ++I) {
      vector<ProductProvenance> ppv;
      ProductProvenance const* prov = nullptr;
      {
        auto p_ppv = &ppv;
        provenanceBranch_->SetAddress(&p_ppv);
        input::getEntry(provenanceBranch_, *I);
        for (auto const& val : ppv) {
          if (val.productID() == bid) {
            prov = &val;
            break;
          }
        }
      }
      // Note: If this is a produced product then it might not be in
      // any of the fragments, this is not an error.
      if (prov != nullptr) {
        if (prov->productStatus() == productstatus::present()) {
          // We found a usable product in the set of fragments,
          // tell our caller to proceed.
          return true;
        }
      }
    }
    // None of the run or subrun fragments have a usable product,
    // tell our caller that he can give up now.
    return false;
  }
}

unique_ptr<EDProduct>
RootDelayedReader::
getProduct_(BranchKey const& bk,
            TypeID const& ty,
            RangeSet& rs) const
{
  auto iter = branches_->find(bk);
  assert(iter != branches_->end());
  input::BranchInfo const& branchInfo = iter->second;
  TBranch* br = branchInfo.productBranch_;
  assert(br != nullptr);
  auto bid = branchInfo.branchDescription_.productID();
  // Note: It is not an error to attempt to delay read a produced
  // run or subrun product because there might be many of them spread
  // across multiple fragments of the same run or subrun which will
  // be combined below.
  if ((branchType_ != InSubRun) && (branchType_ != InRun)) {
    if (branchInfo.branchDescription_.produced()) {
      throw Exception{errors::LogicError, "RootDelayedReader::getProduct_"}
          << "Attempt to delay read a produced product!\n";
    }
  }
  //Note: threading: The configure ref core streamer and the related i/o operations must be done with the source lock held!
  lock_guard<recursive_mutex> lock_input{*art::SharedResourcesRegistry::instance()->getMutexForSource()};
  configureProductIDStreamer(branchIDLists_);
  configureRefCoreStreamer(principal_);
  TClass* cl = TClass::GetClass(ty.typeInfo());
  auto get_product = [this, cl, br](auto entry) {
    unique_ptr<EDProduct> p{static_cast<EDProduct*>(cl->New())};
    EDProduct* pp = p.get();
    br->SetAddress(&pp);
    auto const bytesRead = input::getEntry(br, entry);
    if ((saveMemoryObjectThreshold_ > -1) && (bytesRead > saveMemoryObjectThreshold_)) {
      br->DropBaskets("all");
    }
    return p;
  };
  // Retrieve first product
  auto result = get_product(entrySet_[0]);
  if ((branchType_ != InSubRun) && (branchType_ != InRun)) {
    // Not a run or subrun product, all done.
    configureProductIDStreamer();
    configureRefCoreStreamer();
    return result;
  }
  //
  //  Retrieve and combine multiple Run/SubRun products as needed (this is aggregation!).
  //
  // Products from files that did not support RangeSets are
  // assigned RangeSets that correspond to the entire run/subrun.
  if (fileFormatVersion_.value_ < 9) {
    if (branchType_ == InRun) {
      rs = RangeSet::forRun(eventID_.runID());
    }
    else {
      rs = RangeSet::forSubRun(eventID_.subRunID());
    }
    configureProductIDStreamer();
    configureRefCoreStreamer();
    return result;
  }
  // Fetch the provenance for the first product.
  vector<ProductProvenance> ppv;
  unique_ptr<ProductProvenance const> prov;
  {
    auto p_ppv = &ppv;
    provenanceBranch_->SetAddress(&p_ppv);
    input::getEntry(provenanceBranch_, entrySet_[0]);
    for (auto const& val : ppv) {
      if (val.productID() == bid) {
        prov.reset(new ProductProvenance(val));
        break;
      }
    }
  }
  // Note: Cannot make this assert here because it can be so that the first product
  //       is a dummy wrapper with the present flag false, and no provenance has
  //       been written (How???), but the second product is present and has provenance.
  //assert((prov.get() != nullptr) && "Could not find provenance for this Run/SubRun product!");
  // Unfortunately, we cannot use detail::resolveRangeSetInfo in
  // this case because products that represent a full (Sub)Run are
  // allowed to be duplicated in an input file.  The behavior in
  // such a case is a NOP.
  RangeSet mergedRangeSet = detail::resolveRangeSet(db_, "SomeInput"s, branchType_, result->getRangeSetID());
  // Note: If the mergedRangeSet is invalid here that means the first product was a dummy created
  //       by RootOutputFile to prevent double-counting when combining products.  Or the user is
  //       playing games that are going to hurt them by intentionally created a product with an
  //       invalid range set.
  // Note: Also in that case we have a product status of unknown which we may have to replace later.
  for (auto it = entrySet_.cbegin() + 1, e = entrySet_.cend(); it != e; ++it) {
    auto p = get_product(*it);
    vector<ProductProvenance> new_ppv;
    unique_ptr<ProductProvenance const> new_prov;
    {
      auto p_ppv = &new_ppv;
      provenanceBranch_->SetAddress(&p_ppv);
      input::getEntry(provenanceBranch_, *it);
      for (auto const& val : new_ppv) {
        if (val.productID() == bid) {
          new_prov.reset(new ProductProvenance(val));
          break;
        }
      }
    }
    //assert((new_prov.get() != nullptr) && "Could not find provenance for this Run/SubRun product!");
    //auto const id = p->getRangeSetID();
    RangeSet const& newRS = detail::resolveRangeSet(db_, "SomeInput"s, branchType_, p->getRangeSetID());
    if (!mergedRangeSet.is_valid() && !newRS.is_valid()) {
      // Both range sets are invalid, do nothing.
      // RootOutputFile creates this situation to prevent double-counting when combining products.
      // Possibly can also happen when a produced product is not produced or when a product is dropped?
    }
    else if (mergedRangeSet.is_valid() && !newRS.is_valid()) {
      //FIXME: Can a neverCreated or dropped product have a valid range set?
      //assert(prov->productStatus() == productstatus::present());
      // Our current merged range set is valid and the next one is invalid, do nothing.
      // RootOutputFile creates this situation to prevent double-counting when combining products.
      // Possibly can also happen when a produced product is not produced or when a product is dropped?
    }
    else if (!mergedRangeSet.is_valid() && newRS.is_valid()) {
      // We finally have a valid range set to use.
      //FIXME: Can a neverCreated or dropped product have a valid range set?
      //assert(new_prov->productStatus() == productstatus::present());
      mergedRangeSet = newRS;
      std::swap(result, p);
      // Replace the provenance.
      // Note: We do not worry about productstatus::unknown() here because newRS is valid.
      principal_->insert_pp(make_unique<ProductProvenance const>(*new_prov));
      prov = move(new_prov);
    }
    else if (art::disjoint_ranges(mergedRangeSet, newRS)) {
      // Old and new range sets can be combined, do it.
      //FIXME: Can a neverCreated or dropped product have a valid range set?
      //assert(prov->productStatus() == productstatus::present());
      //assert(new_prov->productStatus() == productstatus::present());
      result->combine(p.get());
      mergedRangeSet.merge(newRS);
      //FIXME: What do we do about provenance??? What if one product has status present and the other neverCreated or dropped?
      //FIXME: Possibly cannot happen because in the case that the new provenance is not present it would have
      //FIXME: an invalid range set?
      // Note: We do not worry about productstatus::unknown() here because newRS is valid.
      //assert((prov->productStatus() == new_prov->productStatus()) && "Unequal product status when merging range sets!");
    }
    else if (art::same_ranges(mergedRangeSet, newRS)) {
      // The ranges are the same, so the behavior is a NOP.  If
      // the stakeholders decide that products with the same
      // ranges should be checked for equality, the condition
      // will be added here.
      //FIXME: Can a neverCreated or dropped product have a valid range set?
      //assert(prov->productStatus() == productstatus::present());
      //assert(new_prov->productStatus() == productstatus::present());
      //FIXME: What do we do about provenance??? What if one product has status present and the other neverCreated or dropped?
      //FIXME: Possibly cannot happen because in the case that the new provenance is not present it would have
      //FIXME: an invalid range set?
      // Note: We do not worry about productstatus::unknown() here because newRS is valid.
      //assert((prov->productStatus() == new_prov->productStatus()) && "Unequal product status when products have identical range sets!");
    }
    else if (art::overlapping_ranges(mergedRangeSet, newRS)) {
      throw Exception{errors::ProductCannotBeAggregated, "RootDelayedReader::getProduct_"}
          << "\nThe following ranges corresponding to the product:\n"
          << "   '" << bk << "'"
          << "\ncannot be aggregated\n"
          << mergedRangeSet
          << " and\n"
          << newRS
          << "\nPlease contact artists@fnal.gov.\n";
    }
  }
  // Now transfer the calculated mergedRangeSet to the output argument.
  std::swap(rs, mergedRangeSet);
  // And now we are done.
  configureProductIDStreamer();
  configureRefCoreStreamer();
  return result;
}

// FIXME: This should be a member of RootInputFileSequence.
int
RootDelayedReader::
openNextSecondaryFile_(int const idx)
{
  // idx being a number we can actually use is a precondition of this
  // function.
  assert(!(idx < 0));
  // Note:
  //
  // Return code of -2 means stop, -1 means event-not-found,
  // otherwise 0 for success.
  //
  auto const& sfnm = primaryFile_->secondaryFileNames();
  assert(!(static_cast<decltype(sfnm.size())>(idx) > sfnm.size()));
  if (sfnm.empty()) { // No configured secondary files.
    return -2;
  }
  auto const& sf = primaryFile_->secondaryFiles();
  if (static_cast<decltype(sfnm.size())>(idx) == sfnm.size()) {
    // We're done.
    return -2;
  }
  if (!sf[idx]) {
    primaryFile_->openSecondaryFile(idx);
  }
  switch (branchType_) {
    case InEvent: {
        if (!sf[idx]->readEventForSecondaryFile(eventID_)) {
          return -1;
        }
      }
      break;
    case InSubRun: {
        if (!sf[idx]->readSubRunForSecondaryFile(eventID_.subRunID())) {
          return -1;
        }
      }
      break;
    case InRun: {
        if (!sf[idx]->readRunForSecondaryFile(eventID_.runID())) {
          return -1;
        }
      }
      break;
    default: {
        assert(false && "RootDelayedReader encountered an unknown BranchType!");
        return -2;
      }
  }
  return 0;
}

} // namespace art
