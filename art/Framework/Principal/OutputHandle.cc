#include "art/Framework/Principal/OutputHandle.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "cetlib/exception.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

using namespace std;

namespace art {

OutputHandle::
~OutputHandle()
{
}

OutputHandle::
OutputHandle(EDProduct const* prod, BranchDescription const* desc, cet::exempt_ptr<ProductProvenance const> productProvenance,
             RangeSet const& rs)
  : desc_{desc}
  , productProvenance_{productProvenance}
  , wrap_{prod}
  , rangeOfValidity_{rs}
{
}

///Used when the attempt to get the data failed
OutputHandle::
OutputHandle(RangeSet const& rs)
  : desc_{nullptr}
  , productProvenance_{nullptr}
  , wrap_{nullptr}
  , rangeOfValidity_{rs}
{
}

bool
OutputHandle::
isValid() const
{
  return desc_ && productProvenance_ && wrap_;
}

BranchDescription const*
OutputHandle::
desc() const
{
  return desc_;
}

ProductProvenance const*
OutputHandle::
productProvenance() const
{
  return productProvenance_.get();
}

EDProduct const*
OutputHandle::
wrapper() const
{
  return wrap_;
}

RangeSet const&
OutputHandle::
rangeOfValidity() const
{
  return rangeOfValidity_;
}

void
OutputHandle::
swap(OutputHandle& other)
{
  using std::swap;
  swap(wrap_, other.wrap_);
  swap(desc_, other.desc_);
  swap(productProvenance_, other.productProvenance_);
}

void
swap(OutputHandle& a, OutputHandle& b)
{
  a.swap(b);
}

} // namespace art

