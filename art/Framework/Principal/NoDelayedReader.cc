#include "art/Framework/Principal/NoDelayedReader.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Utilities/Exception.h"

#include <memory>

namespace art {

NoDelayedReader::
~NoDelayedReader() noexcept
{
}

NoDelayedReader::
NoDelayedReader()
  : DelayedReader()
{
}

std::unique_ptr<EDProduct>
NoDelayedReader::
getProduct_(BranchKey const& k, TypeID const&, RangeSet&) const
{
  throw Exception(errors::LogicError, "NoDelayedReader")
      << "getProduct() called for branchkey: "
      << k
      << "\n";
}

} // namespace art

