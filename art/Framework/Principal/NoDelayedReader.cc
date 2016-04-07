#include "art/Framework/Principal/NoDelayedReader.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Utilities/Exception.h"

namespace art {
  NoDelayedReader::~NoDelayedReader() {}

  std::unique_ptr<EDProduct>
  NoDelayedReader::getProduct_(BranchKey const& k,
                               art::TypeID const&,
                               ProductRangeSetLookup&) const {
    throw art::Exception(errors::LogicError,"NoDelayedReader")
      << "getProduct() called for branchkey: " << k << "\n";
  }
}
