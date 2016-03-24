/*----------------------------------------------------------------------
----------------------------------------------------------------------*/

#include "art/Framework/Principal/NoDelayedReader.h"
#include "canvas/Persistency/Provenance/BranchID.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Utilities/Exception.h"

namespace art {
  NoDelayedReader::~NoDelayedReader() {}

  std::unique_ptr<EDProduct>
  NoDelayedReader::getProduct_(BranchKey const& k, art::TypeID const &) const {
    throw art::Exception(errors::LogicError,"NoDelayedReader")
      << "getProduct() called for branchkey: " << k << "\n";
  }

  RangeSet const&
  NoDelayedReader::getRangeSet_(BranchID const& bid) const {
    throw art::Exception(errors::LogicError,"NoDelayedReader")
      << "getRangeSet() called for BranchID: " << bid << "\n";
  }
}
