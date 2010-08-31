#include "art/Version/GetReleaseVersion.h"
#include <cassert>

int main()
{
  assert(edm::getReleaseVersion() == std::string("PROJECT_VERSION"));
}
