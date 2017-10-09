#include "art/Framework/Principal/NoDelayedReader.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Utilities/Exception.h"

namespace art {
  std::unique_ptr<EDProduct>
  NoDelayedReader::getProduct_(BranchKey const& k,
                               art::TypeID const&,
                               RangeSet&) const
  {
    throw art::Exception(errors::LogicError, "NoDelayedReader")
      << "getProduct() called for branchkey: " << k << "\n";
  }
}
