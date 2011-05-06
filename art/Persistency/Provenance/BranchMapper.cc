#include "art/Persistency/Provenance/BranchMapper.h"

/*
  BranchMapper

*/

namespace art {
  BranchMapper::BranchMapper() :
    entryInfoSet_(),
    nextMapper_(),
    delayedRead_(false)
  { }

  BranchMapper::BranchMapper(bool delayedRead) :
    entryInfoSet_(),
    nextMapper_(),
    delayedRead_(delayedRead)
  { }

  void
  BranchMapper::readProvenance() const {
    if (delayedRead_) {
      delayedRead_ = false;
      readProvenance_();
    }
  }

  void
  BranchMapper::insert(ProductProvenance const& entryInfo) {
    readProvenance();
    entryInfoSet_.insert(entryInfo);
  }

  std::shared_ptr<ProductProvenance>
  BranchMapper::branchToEntryInfo(BranchID const& bid) const {
    readProvenance();
    ProductProvenance ei(bid);
    eiSet::const_iterator it = entryInfoSet_.find(ei);
    if (it == entryInfoSet_.end()) {
      if (nextMapper_) {
	return nextMapper_->branchToEntryInfo(bid);
      } else {
	return std::shared_ptr<ProductProvenance>();
      }
    }
    return std::shared_ptr<ProductProvenance>(new ProductProvenance(*it));
  }

}
