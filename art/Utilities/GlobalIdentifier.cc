#include "art/Utilities/GlobalIdentifier.h"
#include "art/Utilities/Guid.h"

namespace art {
  std::string
  createGlobalIdentifier() {
    Guid guid;
    Guid::create(guid);
    return guid.toString();
  }
}
