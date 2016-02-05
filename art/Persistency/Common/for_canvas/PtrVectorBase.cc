#include "canvas/Persistency/Common/PtrVectorBase.h"

#include <utility>

void
art::PtrVectorBase::fillPtrs() const {
  if (indicies_.size() ==  0) return; // Empty or already done.
  fill_from_offsets(indicies_);

  using std::swap;
  indices_t tmp;
  swap(indicies_, tmp); // Zero -- finished with these.
}
