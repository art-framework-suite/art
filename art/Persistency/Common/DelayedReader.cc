#include "art/Persistency/Common/DelayedReader.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/ProductProvenance.h"

using namespace std;

namespace art {

  DelayedReader::~DelayedReader() noexcept = default;

  DelayedReader::DelayedReader() = default;

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

  void DelayedReader::setPrincipal_(cet::exempt_ptr<Principal>) {}

  vector<ProductProvenance>
  DelayedReader::readProvenance() const
  {
    return readProvenance_();
  }

  vector<ProductProvenance>
  DelayedReader::readProvenance_() const
  {
    vector<ProductProvenance> ret;
    return ret;
  }

  bool
  DelayedReader::isAvailableAfterCombine(ProductID pid) const
  {
    return isAvailableAfterCombine_(pid);
  }

  bool DelayedReader::isAvailableAfterCombine_(ProductID) const
  {
    return false;
  }

  int
  DelayedReader::readFromSecondaryFile(int idx)
  {
    return readFromSecondaryFile_(idx);
  }

  int
  DelayedReader::readFromSecondaryFile_(int /*idx*/)
  {
    return -2;
  }

} // namespace art
