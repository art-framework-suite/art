#include "art/Framework/Principal/NoDelayedReader.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Utilities/Exception.h"

#include <memory>

namespace art {

  NoDelayedReader::~NoDelayedReader() noexcept = default;
  NoDelayedReader::NoDelayedReader() = default;

  std::unique_ptr<EDProduct>
  NoDelayedReader::getProduct_(Group const*,
                               ProductID const pid,
                               RangeSet&) const
  {
    throw Exception(errors::LogicError, "NoDelayedReader")
      << "getProduct() called for ProductID: " << pid << '\n';
  }

} // namespace art
