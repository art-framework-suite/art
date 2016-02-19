#include "art/Framework/IO/Root/RootDelayedReader.h"
// vim: sw=2:

#include "canvas/Persistency/Common/RefCoreStreamer.h"
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
RootDelayedReader(input::EntryNumber const& entryNumber,
                  shared_ptr<input::BranchMap const> branches,
                  shared_ptr<TFile const> filePtr,
                  int64_t saveMemoryObjectThreshold,
                  cet::exempt_ptr<RootInputFile> primaryFile,
                  BranchType branchType, EventID eID)
  : entryNumber_(entryNumber)
  , branches_(branches)
  , filePtr_(filePtr)
  , saveMemoryObjectThreshold_(saveMemoryObjectThreshold)
  , nextReader_()
  , primaryFile_(primaryFile)
  , branchType_(branchType)
  , eventID_(eID)
{
}

void
RootDelayedReader::
setGroupFinder_(cet::exempt_ptr<EDProductGetterFinder const> groupFinder)
{
  groupFinder_ = groupFinder;
}

void
RootDelayedReader::
mergeReaders_(shared_ptr<DelayedReader> other)
{
  nextReader_ = other;
}

unique_ptr<EDProduct>
RootDelayedReader::
getProduct_(BranchKey const& bk, TypeID const& ty) const
{
  auto iter = branches_->find(bk);
  if (iter == branches_->end()) {
    assert(nextReader_);
    return nextReader_->getProduct(bk, ty);
  }
  input::BranchInfo const& branchInfo = iter->second;
  TBranch* br = branchInfo.productBranch_;
  if (br == nullptr) {
    assert(nextReader_);
    return nextReader_->getProduct(bk, ty);
  }
  configureRefCoreStreamer(groupFinder_);
  TClass* cl(TClass::GetClass(ty.typeInfo()));
  // FIXME: This code should be resurrected when ROOT is capable of
  // registering ioread rules for instantiations of class templates
  // using typdefs.
#ifdef ROOT_CAN_REGISTER_IOREADS_PROPERLY
  TBranchElement* be(dynamic_cast<TBranchElement*>(br));
  if (be->GetClass() != cl) {
    // Need to make sure we're calling the correct streamer.
    be->SetTargetClass(cl->GetName());
  }
#endif
  unique_ptr<EDProduct> p(static_cast<EDProduct*>(cl->New()));
  EDProduct* pp = p.get();
  br->SetAddress(&pp);
  auto const bytesRead = input::getEntry(br, entryNumber_);
  if ((saveMemoryObjectThreshold_ > -1) &&
      (bytesRead > saveMemoryObjectThreshold_)) {
    br->DropBaskets("all");
  }
  configureRefCoreStreamer();
  //br->SetAddress(0);
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

