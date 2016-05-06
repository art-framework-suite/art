#include "art/Framework/IO/Root/RootDelayedReader.h"
// vim: sw=2:

#include "TBranch.h"
#include "TBranchElement.h"
#include "TClass.h"
#include "art/Framework/IO/Root/RootInputFile.h"
#include "art/Framework/IO/Root/RootTree.h"
#include "art/Framework/IO/Root/detail/resolveRangeSet.h"
#include "canvas/Persistency/Common/RefCoreStreamer.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/crc32.h"

#include <cassert>

using namespace std;

namespace art {

  RootDelayedReader::
  RootDelayedReader(FileFormatVersion const version,
                    sqlite3* db,
                    std::vector<input::EntryNumber> const& entrySet,
                    shared_ptr<input::BranchMap const> branches,
                    cet::exempt_ptr<RootTree> tree,
                    int64_t const saveMemoryObjectThreshold,
                    cet::exempt_ptr<RootInputFile> primaryFile,
                    BranchType const branchType,
                    EventID const eID)
    : fileFormatVersion_{version}
    , db_{db}
    , entrySet_{entrySet}
    , branches_{branches}
    , tree_{tree}
    , saveMemoryObjectThreshold_{saveMemoryObjectThreshold}
    , primaryFile_{primaryFile}
    , branchType_{branchType}
    , eventID_{eID}
  {}

  void
  RootDelayedReader::
  setGroupFinder_(cet::exempt_ptr<EDProductGetterFinder const> groupFinder)
  {
    groupFinder_ = groupFinder;
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
    TBranch* br {branchInfo.productBranch_};
    assert(br != nullptr);

    configureRefCoreStreamer(groupFinder_);
    TClass* cl {TClass::GetClass(ty.typeInfo())};

    auto get_product = [this, cl, br](auto entry){
      tree_->setEntryNumber(entry);
      unique_ptr<EDProduct> p {static_cast<EDProduct*>(cl->New())};
      EDProduct* pp {p.get()};
      br->SetAddress(&pp);
      auto const bytesRead = input::getEntry(br, entry);
      if ((saveMemoryObjectThreshold_ > -1) &&
          (bytesRead > saveMemoryObjectThreshold_)) {
        br->DropBaskets("all");
      }
      return p;
    };

    // Retrieve first product
    auto result = get_product(entrySet_[0]);


    // Retrieve and aggregate subsequent products (if they exist)
    if (branchType_ == InSubRun || branchType_ == InRun) {

      // Products from files that did not support RangeSets are
      // assigned RangeSets that correspond to the entire run/subrun.
      if (fileFormatVersion_.value_ < 9) {
        if (branchType_ == InRun) {
          auto fullRS = RangeSet::forRun(eventID_.runID());
          std::swap(rs, fullRS);
        }
        else {
          auto fullRS = RangeSet::forSubRun(eventID_.subRunID());
          std::swap(rs, fullRS);
        }
        return result;
      }

      std::set<unsigned> seenIDs;
      seenIDs.insert(result->getRangeSetID());
      RangeSet mergedRangeSet = detail::resolveRangeSet(db_,
                                                        "SomeInput"s,
                                                        branchType_,
                                                        result->getRangeSetID());

      for(auto it = entrySet_.cbegin()+1, e = entrySet_.cend(); it!= e; ++it) {
        auto p = get_product(*it);
        auto const id = p->getRangeSetID();

        if (!seenIDs.insert(id).second) continue; // Skip an already-seen product;
                                                  // double-counting is bad.

        RangeSet const& newRS = detail::resolveRangeSet(db_, "SomeInput"s, branchType_, id);
        if (!mergedRangeSet.is_valid() && newRS.is_valid()) {
          mergedRangeSet = newRS;
          std::swap(result, p);
        }
        else if (art::disjoint_ranges(mergedRangeSet, newRS)) {
          result->combine(p.get());
          mergedRangeSet.merge(newRS);
        }
        else if (art::same_ranges(mergedRangeSet, newRS)) {
          // The ranges are the same, so the behavior is a NOP.
          // However, we will probably never get here because of the
          // seenIDs set, which prevents from duplicate aggregation.
          // If the stakeholders decide that products with the same
          // ranges should be checked for equality, then the seenIDs
          // set needs to go away, and an extra condition will be
          // added here.
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
        // NOP for when both RangeSets are invalid

      }
      mergedRangeSet.collapse(); // Must collapse the range!  It sets
                                 // the checksum.  Yeah...I don't like
                                 // it either. -KJK
      std::swap(rs, mergedRangeSet);
    }

    configureRefCoreStreamer();
    return result;
  }

  // FIXME: This should be a member of RootInputFileSequence.
  int
  RootDelayedReader::
  openNextSecondaryFile_(int idx)
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
