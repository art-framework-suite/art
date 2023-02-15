#include "art/Framework/Principal/DelayedReader.h"
#include "art/Framework/Principal/Principal.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/ProductProvenance.h"

using namespace std;

namespace art {

  DelayedReader::DelayedReader() = default;
  DelayedReader::~DelayedReader() noexcept = default;

  std::unique_ptr<EDProduct>
  DelayedReader::getProduct(Group const* grp,
                            ProductID const pid,
                            RangeSet& rs) const
  {
    return getProduct_(grp, pid, rs);
  }

  void
  DelayedReader::setPrincipal(cet::exempt_ptr<Principal> principal)
  {
    setPrincipal_(principal);
  }

  void
  DelayedReader::setPrincipal_(cet::exempt_ptr<Principal>)
  {}

  vector<ProductProvenance>
  DelayedReader::readProvenance() const
  {
    return readProvenance_();
  }

  vector<ProductProvenance>
  DelayedReader::readProvenance_() const
  {
    return {};
  }

  bool
  DelayedReader::isAvailableAfterCombine(ProductID pid) const
  {
    return isAvailableAfterCombine_(pid);
  }

  bool
  DelayedReader::isAvailableAfterCombine_(ProductID) const
  {
    return false;
  }

  std::unique_ptr<Principal>
  DelayedReader::readFromSecondaryFile(int& idx)
  {
    return readFromSecondaryFile_(idx);
  }

  std::unique_ptr<Principal>
  DelayedReader::readFromSecondaryFile_(int& /*idx*/)
  {
    return nullptr;
  }

} // namespace art
