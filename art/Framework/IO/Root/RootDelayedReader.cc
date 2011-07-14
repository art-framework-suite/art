#include "art/Framework/IO/Root/RootDelayedReader.h"

#include "art/Persistency/Common/RefCoreTransientStreamer.h"
#include "art/Persistency/Provenance/BranchDescription.h"

#include "TBranch.h"
#include "TClass.h"


namespace art {

  RootDelayedReader::RootDelayedReader(EntryNumber const& entry,
      std::shared_ptr<BranchMap const> bMap,
      std::shared_ptr<TFile const> filePtr,
      bool oldFormat) :
   entryNumber_(entry),
   branches_(bMap),
   filePtr_(filePtr),
   nextReader_(),
   oldFormat_(oldFormat) {}

  RootDelayedReader::~RootDelayedReader() {}

  std::auto_ptr<EDProduct>
  RootDelayedReader::getProduct_(BranchKey const& k, EDProductGetter const* ep) const {
    iterator iter = branchIter(k);
    if (!found(iter)) {
      assert(nextReader_);
      return nextReader_->getProduct(k, ep);
    }
    input::BranchInfo const& branchInfo = getBranchInfo(iter);
    TBranch *br = branchInfo.productBranch_;
    if (br == 0) {
      assert(nextReader_);
      return nextReader_->getProduct(k, ep);
    }
    configureRefCoreTransientStreamer(ep);
    TClass *cp = TClass::GetClass(branchInfo.branchDescription_.wrappedCintName().c_str());
    std::auto_ptr<EDProduct> p(static_cast<EDProduct *>(cp->New()));
    EDProduct *pp = p.get();
    br->SetAddress(&pp);
    input::getEntry(br, entryNumber_);
    configureRefCoreTransientStreamer();
    return p;
  }

}  // art
