#include "art/Framework/IO/Root/RootDelayedReader.h"
// vim: sw=2:

#include "art/Framework/IO/Root/RefCoreStreamer.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Utilities/TypeID.h"
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
setGroupFinder_(cet::exempt_ptr<EventPrincipal const> groupFinder)
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

int
RootDelayedReader::
openNextSecondaryFile_(int idx)
{
  // Note:
  //
  // Return code of -2 means stop, -1 means event-not-found,
  // otherwise 0 for success.
  //
  auto const& sfnm = primaryFile_->secondaryFileNames();
  if (sfnm.empty()) {
    return -2;
  }
  auto const& sf = primaryFile_->secondaryFiles();
  if (idx < 0) {
    // FIXME: Throw an impossible error here!
    return -2;
  }
  if (static_cast<decltype(sfnm.size())>(idx) == sfnm.size()) {
    return -2;
  }
  if (static_cast<decltype(sfnm.size())>(idx) > sfnm.size()) {
    // FIXME: Thow an impossible error here!
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
        // FIXME: Throw unknown branch type error here!
        return -2;
      }
      break;
  }
  return 0;
}

} // namespace art

