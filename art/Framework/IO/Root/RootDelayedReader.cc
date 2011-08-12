#include "art/Framework/IO/Root/RootDelayedReader.h"

#include "art/Framework/IO/Root/RefCoreStreamer.h"
#include "art/Persistency/Provenance/BranchDescription.h"

#include "TBranch.h"
#include "TClass.h"


art::RootDelayedReader::
RootDelayedReader(EntryNumber const& entry,
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
art::RootDelayedReader::getProduct_(BranchKey const& k) const {
  iterator iter = branchIter(k);
  if (!found(iter)) {
    assert(nextReader_);
    return nextReader_->getProduct(k);
  }
  input::BranchInfo const& branchInfo = getBranchInfo(iter);
  TBranch *br = branchInfo.productBranch_;
  if (br == 0) {
    assert(nextReader_);
    return nextReader_->getProduct(k);
  }
  configureRefCoreStreamer(groupFinder_);
  TClass *cp = TClass::GetClass(branchInfo.branchDescription_.wrappedCintName().c_str());
  std::auto_ptr<EDProduct> p(static_cast<EDProduct *>(cp->New()));
  EDProduct *pp = p.get();
  br->SetAddress(&pp);
  input::getEntry(br, entryNumber_);
  configureRefCoreStreamer();
  return p;
}

void
art::RootDelayedReader::setGroupFinder_(cet::exempt_ptr<EventPrincipal const> groupFinder) {
  groupFinder_ = groupFinder;
}
