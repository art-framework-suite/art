#include "art/Persistency/Common/PtrVectorBase.h"

#include <utility>

void
art::PtrVectorBase::fillPtrs() const {
  if (indices_.size() ==  0) return; // Empty or already done.
  fill_from_offsets(indices_);

  using std::swap;
  indices_t tmp;
  swap(indices_, tmp); // Zero -- finished with these.
}
