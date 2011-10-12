#include "art/Framework/IO/Root/RootDelayedReader.h"

#include "art/Framework/IO/Root/RefCoreStreamer.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Utilities/TypeID.h"

#include "TBranch.h"
#include "TBranchElement.h"
#include "TClass.h"


art::RootDelayedReader::
RootDelayedReader(EntryNumber const & entry,
                  std::shared_ptr<BranchMap const> bMap,
                  std::shared_ptr<TFile const> filePtr,
                  bool oldFormat) :
  entryNumber_(entry),
  branches_(bMap),
  filePtr_(filePtr),
  nextReader_(),
  oldFormat_(oldFormat) {}

art::RootDelayedReader::~RootDelayedReader() {}

std::auto_ptr<art::EDProduct>
art::RootDelayedReader::getProduct_(BranchKey const & k, art::TypeID const & wrapper_type) const
{
  iterator iter = branchIter(k);
  if (!found(iter)) {
    assert(nextReader_);
    return nextReader_->getProduct(k, wrapper_type);
  }
  input::BranchInfo const & branchInfo = getBranchInfo(iter);
  TBranch * br = branchInfo.productBranch_;
  if (br == 0) {
    assert(nextReader_);
    return nextReader_->getProduct(k, wrapper_type);
  }
  configureRefCoreStreamer(groupFinder_);
  TClass * cl(TClass::GetClass(wrapper_type.typeInfo()));
  // FIXME: This code should be resurrected when ROOT is capable of
  // registering ioread rules for instantiations of class templates
  // using typdefs.
#ifdef ROOT_CAN_REGISTER_IOREADS_PROPERLY
  TBranchElement & be(dynamic_cast<TBranchElement &>(*br));
  if (be.GetClass() != cl) {
    // Need to make sure we're calling the correct streamer.
    be.SetTargetClass(cl->GetName());
  }
#endif
  std::auto_ptr<EDProduct> p(static_cast<EDProduct *>(cl->New()));
  EDProduct * pp = p.get();
  br->SetAddress(&pp);
  input::getEntry(br, entryNumber_);
  configureRefCoreStreamer();
  return p;
}

void
art::RootDelayedReader::setGroupFinder_(cet::exempt_ptr<EventPrincipal const> groupFinder)
{
  groupFinder_ = groupFinder;
}
