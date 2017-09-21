#include "art/Persistency/Common/DelayedReader.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <vector>

using namespace std;

namespace art {

DelayedReader::
~DelayedReader() noexcept
{
}

DelayedReader::
DelayedReader()
{
}

std::unique_ptr<EDProduct>
DelayedReader::
getProduct(ProductID const pid, TypeID const& wrapper_type, RangeSet& rs) const
{
  return getProduct_(pid, wrapper_type, rs);
}

void
DelayedReader::
setPrincipal(cet::exempt_ptr<Principal> principal)
{
  setPrincipal_(principal);
}

void
DelayedReader::
setPrincipal_(cet::exempt_ptr<Principal>)
{
}

vector<ProductProvenance>
DelayedReader::
readProvenance() const
{
  return readProvenance_();
}

vector<ProductProvenance>
DelayedReader::
readProvenance_() const
{
  vector<ProductProvenance> ret;
  return ret;
}

bool
DelayedReader::
isAvailableAfterCombine(ProductID pid) const
{
  return isAvailableAfterCombine_(pid);
}

bool
DelayedReader::
isAvailableAfterCombine_(ProductID) const
{
  return false;
}

int
DelayedReader::
openNextSecondaryFile(int idx)
{
  return openNextSecondaryFile_(idx);
}

int
DelayedReader::
openNextSecondaryFile_(int /*idx*/)
{
  return -2;
}

} // namespace art
