#include "art/Framework/IO/Root/RootDelayedReader.h"
// vim: sw=2:

#include "TBranch.h"
#include "TBranchElement.h"
#include "TClass.h"
#include "art/Framework/IO/Root/RootInputFile.h"
#include "art/Framework/IO/Root/RootTree.h"
#include "art/Framework/IO/Root/detail/getFileContributors.h"
#include "canvas/Persistency/Common/RefCoreStreamer.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/crc32.h"

#include <cassert>

using namespace std;

namespace {

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
  getProduct_(BranchKey const& bk,
              TypeID const& ty,
              ProductRangeSetLookup& productRangeSetLookup) const
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
      std::set<unsigned> seenIDs;
      seenIDs.insert(result->getRangeSetID());
      RangeSet mergedRangeSet = detail::getContributors(db_,
                                                        "SomeInput"s,
                                                        branchType_,
                                                        result->getRangeSetID());

      for(auto it = entrySet_.cbegin()+1, e = entrySet_.cend(); it!= e; ++it) {
        auto p = get_product(*it);
        auto const id = p->getRangeSetID();

        if (!seenIDs.insert(id).second) continue; // Skip an already-seen product;
                                                  // double-counting is bad.

        RangeSet const& rs = detail::getContributors(db_, "SomeInput"s, branchType_, id);
        if (art::are_disjoint(mergedRangeSet, rs)) {
          result->combine(p.get());
          mergedRangeSet.merge(rs);
        }
      }
      productRangeSetLookup.emplace(bk, RangeSet{mergedRangeSet});
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
