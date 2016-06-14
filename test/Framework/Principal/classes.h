#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Wrapper.h"

#include <string>

namespace art {
  template class Wrapper<art::Assns<size_t, std::string, void> >;
  template class Wrapper<art::Assns<std::string, size_t, void> >;
}
