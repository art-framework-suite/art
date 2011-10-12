/*----------------------------------------------------------------------
----------------------------------------------------------------------*/

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Utilities/Exception.h"

namespace art {
  NoDelayedReader::~NoDelayedReader() {}

  std::auto_ptr<EDProduct>
  NoDelayedReader::getProduct_(BranchKey const & k, art::TypeID const &) const
  {
    throw art::Exception(errors::LogicError, "NoDelayedReader")
        << "getProduct() called for branchkey: " << k << "\n";
  }
}
