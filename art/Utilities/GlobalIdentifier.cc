#include "art/Utilities/GlobalIdentifier.h"
#include "Guid.h"

namespace art {
  std::string
  createGlobalIdentifier() {
    Guid guid;
    Guid::create(guid);
    return guid.toString();
  }
}
