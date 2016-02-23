#include "art/Framework/IO/Root/RootDelayedReader.h"
// vim: sw=2:

#include "canvas/Framework/IO/Root/RefCoreStreamer.h"
#include "canvas/Framework/IO/Root/RootInputFile.h"
#include "canvas/Framework/IO/Root/RootTree.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Utilities/TypeID.h"
#include "TBranch.h"
#include "TBranchElement.h"
#include "TClass.h"
#include <cassert>

using namespace std;

namespace art {

RootDelayedReader::
~RootDelayedReader()
{
}

RootDelayedReader::
RootDelayedReader(std::vector<input::EntryNumber> const& entrySet,
                  shared_ptr<input::BranchMap const> branches,
                  shared_ptr<TFile const> filePtr [[gnu::unused]],
                  cet::exempt_ptr<RootTree> tree,
                  int64_t saveMemoryObjectThreshold,
                  cet::exempt_ptr<RootInputFile> primaryFile,
                  BranchType branchType, EventID eID)
  : entrySet_{entrySet}
  , branches_{branches}
  //  , filePtr_{filePtr}
  , tree_{tree}
  , saveMemoryObjectThreshold_{saveMemoryObjectThreshold}
  , primaryFile_{primaryFile}
  , branchType_{branchType}
  , eventID_{eID}
{
}

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
  unique_ptr<EDProduct> p {nullptr};

  // Aggregate the products

  auto fill_product = [this, cl, br](auto entry){
    tree_->setEntryNumber(entry);
    decltype(p) tmp {static_cast<EDProduct*>(cl->New())};
    EDProduct* pp {tmp.get()};
    br->SetAddress(&pp);
    auto const bytesRead = input::getEntry(br, entry);
    if ((saveMemoryObjectThreshold_ > -1) &&
        (bytesRead > saveMemoryObjectThreshold_)) {
      br->DropBaskets("all");
    }
    return tmp;
  };

  if (entrySet_.size() == 1ul) {
    p = fill_product(entrySet_[0]);
  }
  else {
    for(auto const entry : entrySet_) {
      auto tmp = fill_product(entry);
      p->combine(tmp.get());
    }
  }

  configureRefCoreStreamer();
  return p;
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
