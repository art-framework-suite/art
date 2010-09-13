#include "art/Persistency/Provenance/ParameterSetBlob.h"

namespace edm {

  std::ostream&
  operator<<(std::ostream& os, ParameterSetBlob const& blob) {
    os << blob.pset_;
    return os;
  }

}  // namespace edm
