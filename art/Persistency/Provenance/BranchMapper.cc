#include "art/Persistency/Provenance/BranchMapper.h"

/*
  BranchMapper

*/

namespace art {
  BranchMapper::BranchMapper() :
    entryInfoSet_(),
    delayedRead_(false)
  { }

  BranchMapper::BranchMapper(bool delayedRead) :
    entryInfoSet_(),
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
    entryInfoSet_.insert(std::make_pair(entryInfo.branchID(), entryInfo));
  }

  std::shared_ptr<ProductProvenance>
  BranchMapper::branchToProductProvenance(BranchID const& bid) const {
    readProvenance();
    eiSet::const_iterator it = entryInfoSet_.find(bid);
    if (it != entryInfoSet_.end()) {
      return std::shared_ptr<ProductProvenance>(new ProductProvenance(it->second));
    } else {
      return std::shared_ptr<ProductProvenance>();
    }
  }

}
