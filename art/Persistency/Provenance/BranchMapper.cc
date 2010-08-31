#include "art/Persistency/Provenance/BranchMapper.h"

/*
  BranchMapper

*/

namespace edm {
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

  boost::shared_ptr<ProductProvenance>
  BranchMapper::branchToEntryInfo(BranchID const& bid) const {
    readProvenance();
    ProductProvenance ei(bid);
    eiSet::const_iterator it = entryInfoSet_.find(ei);
    if (it == entryInfoSet_.end()) {
      if (nextMapper_) {
	return nextMapper_->branchToEntryInfo(bid);
      } else {
	return boost::shared_ptr<ProductProvenance>();
      }
    }
    return boost::shared_ptr<ProductProvenance>(new ProductProvenance(*it));
  }

  BranchID
  BranchMapper::oldProductIDToBranchID_(ProductID const& ) const {
    throw edm::Exception(errors::LogicError)
        << "Internal error:  Illegal call of oldProductIDToBranchID_.\n"
        << "Please report this error to the Framework group\n";
  }
}
