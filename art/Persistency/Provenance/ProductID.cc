#include "art/Persistency/Provenance/ProductID.h"
#include <ostream>

namespace art {
  std::ostream&
  operator<<(std::ostream& os, ProductID const& id) {
    os << id.processIndex() << ":" << id.productIndex();
    return os;
  }

  bool operator<(ProductID const& lh, ProductID const& rh) {
    return lh.processIndex() < rh.processIndex() ||
      (lh.processIndex() == rh.processIndex() && lh.productIndex() < rh.productIndex());
  }
}
