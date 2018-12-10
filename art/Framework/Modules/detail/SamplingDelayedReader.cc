#include "art/Framework/Modules/detail/SamplingDelayedReader.h"
// vim: sw=2:

#include "art/Framework/IO/Root/detail/resolveRangeSet.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Utilities/TypeID.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "canvas/Utilities/uniform_type_name.h"
#include "canvas_root_io/Streamers/ProductIDStreamer.h"
#include "canvas_root_io/Streamers/RefCoreStreamer.h"
#include "cetlib/crc32.h"

#include "TBranch.h"
#include "TBranchElement.h"
#include "TClass.h"

#include <cassert>

using namespace std;

namespace art {
  namespace detail {

    SamplingDelayedReader::SamplingDelayedReader(
      FileFormatVersion const version,
      sqlite3* db,
      std::vector<input::EntryNumber> const& entrySet,
      input::BranchMap const& branches,
      cet::exempt_ptr<TTree> tree,
      int64_t const saveMemoryObjectThreshold,
      cet::exempt_ptr<BranchIDLists const> bidLists,
      BranchType const branchType,
      EventID const& eID,
      bool const compactSubRunRanges)
      : fileFormatVersion_{version}
      , db_{db}
      , entrySet_{entrySet}
      , branches_{branches}
      , tree_{tree}
      , saveMemoryObjectThreshold_{saveMemoryObjectThreshold}
      , branchIDLists_{bidLists}
      , branchType_{branchType}
      , eventID_{eID}
      , compactSubRunRanges_{compactSubRunRanges}
    {}

    void
    SamplingDelayedReader::setGroupFinder_(
      cet::exempt_ptr<EDProductGetterFinder const> groupFinder)
    {
      groupFinder_ = groupFinder;
    }

    unique_ptr<EDProduct>
    SamplingDelayedReader::getProduct_(BranchKey const& bk,
                                       TypeID const& ty,
                                       RangeSet& rs) const
    {
      return getProduct(bk, uniform_type_name(ty.className()), rs);
    }

    unique_ptr<EDProduct>
    SamplingDelayedReader::getProduct(BranchKey const& bk,
                                      std::string const& wrapped_class_name,
                                      RangeSet& rs) const
    {
      auto iter = branches_.find(bk);
      assert(iter != branches_.end());

      input::BranchInfo const& branchInfo = iter->second;
      TBranch* br{branchInfo.productBranch_};
      assert(br != nullptr);

      configureProductIDStreamer(branchIDLists_);
      configureRefCoreStreamer(groupFinder_);
      TClass* cl{TClass::GetClass(wrapped_class_name.c_str())};

      auto get_product = [this, cl, br](auto entry) {
        tree_->LoadTree(entry);
        unique_ptr<EDProduct> p{static_cast<EDProduct*>(cl->New())};
        EDProduct* pp{p.get()};
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
            rs = RangeSet::forRun(eventID_.runID());
          } else {
            rs = RangeSet::forSubRun(eventID_.subRunID());
          }
          configureProductIDStreamer();
          configureRefCoreStreamer();
          return result;
        }

        // Unfortunately, we cannot use detail::resolveRangeSetInfo in
        // this case because products that represent a full (Sub)Run are
        // allowed to be duplicated in an input file.  The behavior in
        // such a case is a NOP.
        RangeSet mergedRangeSet =
          detail::resolveRangeSet(db_,
                                  "SomeInput"s,
                                  branchType_,
                                  result->getRangeSetID(),
                                  compactSubRunRanges_);

        for (auto it = entrySet_.cbegin() + 1, e = entrySet_.cend(); it != e;
             ++it) {
          auto p = get_product(*it);
          auto const id = p->getRangeSetID();

          RangeSet const& newRS = detail::resolveRangeSet(
            db_, "SomeInput"s, branchType_, id, compactSubRunRanges_);
          if (!mergedRangeSet.is_valid() && newRS.is_valid()) {
            mergedRangeSet = newRS;
            std::swap(result, p);
          } else if (art::disjoint_ranges(mergedRangeSet, newRS)) {
            result->combine(p.get());
            mergedRangeSet.merge(newRS);
          } else if (art::same_ranges(mergedRangeSet, newRS)) {
            // The ranges are the same, so the behavior is a NOP.  If
            // the stakeholders decide that products with the same
            // ranges should be checked for equality, the condition
            // will be added here.
          } else if (art::overlapping_ranges(mergedRangeSet, newRS)) {
            throw Exception{errors::ProductCannotBeAggregated,
                            "SamplingDelayedReader::getProduct_"}
              << "\nThe following ranges corresponding to the product:\n"
              << "   '" << bk << "'"
              << "\ncannot be aggregated\n"
              << mergedRangeSet << " and\n"
              << newRS << "\nPlease contact artists@fnal.gov.\n";
          }
          // NOP when both RangeSets are invalid
        }
        std::swap(rs, mergedRangeSet);
      }

      configureProductIDStreamer();
      configureRefCoreStreamer();
      return result;
    }
  } // namespace detail
} // namespace art
