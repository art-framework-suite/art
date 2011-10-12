// ======================================================================
//
// PtrVectorBase: provide PtrVector<T> behavior that's independent of T
//
// ======================================================================

#include "art/Persistency/Common/PtrVectorBase.h"

#include "TBuffer.h"
#include "TClassRef.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/traits.h"
#include "art/Utilities/Exception.h"
#include "cetlib/exception.h"
#include "cpp0x/algorithm"

// Constructor and destructor:

art::PtrVectorBase::PtrVectorBase()
  : core_()
  , indices_()
{}

art::PtrVectorBase::~PtrVectorBase()
{}


void
art::PtrVectorBase::fillPtrs() const
{
  if (indices_.size() ==  0) { return; } // Empty or already done.
  fill_from_offsets(indices_);
  indices_t tmp;
  indices_.swap(tmp); // Zero -- no longer needed.
}

bool
art::PtrVectorBase::operator==(PtrVectorBase const & other) const
{
  return core_ == other.core_;
}
