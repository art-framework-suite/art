#include "art/Framework/IO/Root/RootDelayedReader.h"
// vim: sw=2:

#include "art/Framework/IO/Root/RootInputFile.h"
#include "art/Framework/IO/Root/RootTree.h"
#include "art/Framework/IO/Root/detail/getFileContributors.h"
#include "canvas/Persistency/Common/RefCoreStreamer.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/crc32.h"
#include "TBranch.h"
#include "TBranchElement.h"
#include "TClass.h"
#include <cassert>

using namespace std;

namespace {


  inline unsigned to_id(art::RangeSet const& rs)
  {
    cet::crc32 c{rs.to_compact_string()};
    return c.digest();
  }

  inline auto to_bid(art::BranchKey const& bk)
  {
    return art::BranchID{bk.branchName()};
  }
}

namespace art {

  RootDelayedReader::
  RootDelayedReader(sqlite3* db,
                    std::vector<input::EntryNumber> const& entrySet,
                    shared_ptr<input::BranchMap const> branches,
                    cet::exempt_ptr<RootTree> tree,
                    int64_t const saveMemoryObjectThreshold,
                    cet::exempt_ptr<RootInputFile> primaryFile,
                    BranchType const branchType,
                    EventID const eID)
    : db_{db}
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
  getProduct_(BranchKey const& bk, TypeID const& ty) const
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
    if (branchType_ == InSubRun) {
      std::set<unsigned> seenIDs;
      seenIDs.insert(result->getRangeSetID());
      RangeSet mergedRangeSet = detail::getSubRunContributors(db_,
                                                              "SomeInput"s,
                                                              result->getRangeSetID());

      for(auto it = entrySet_.cbegin()+1, e = entrySet_.cend(); it!= e; ++it) {
        auto p = get_product(*it);
        auto const id = p->getRangeSetID();

        if (!seenIDs.insert(id).second) continue; // Skip an already-seen product;
                                                  // double-counting is bad.

        RangeSet const& rs = detail::getSubRunContributors(db_, "SomeInput"s, id);
        if (art::is_disjoint(mergedRangeSet, rs)) {
          result->combine(p.get());
          mergedRangeSet.merge(rs);
        }
      }
      auto const checksum = mergedRangeSet.checksum();
      rangeSets_.emplace(checksum, mergedRangeSet);
      productRangeSetChecksums_.emplace(to_bid(bk), checksum);
    }

    configureRefCoreStreamer();
    return result;
  }

  RangeSet const&
  RootDelayedReader::getRangeSet_(BranchID const& bid) const
  {
    auto it = productRangeSetChecksums_.find(bid);
    if (it == productRangeSetChecksums_.cend())
      throw art::Exception(art::errors::LogicError,"RootDelayedReader::getRangeSet_")
        << "Could not find range set for BranchID: " << bid << '\n';

    auto const checksum = it->second;
    auto f = rangeSets_.find(checksum);
    assert(f != rangeSets_.cend()); // Based on construction, this must be true.

    return f->second;
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
