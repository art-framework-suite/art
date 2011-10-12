#include "art/Persistency/Provenance/ParameterSetBlob.h"

namespace art {

  std::ostream &
  operator<<(std::ostream & os, ParameterSetBlob const & blob)
  {
    os << blob.pset_;
    return os;
  }

}  // art
