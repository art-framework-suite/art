#include "art/Utilities/ToolMacros.h"

namespace arttest {
  int addOne(int const i)
  {
    return i+1;
  }
}

DEFINE_ART_TOOL_FUNCTION(arttest::addOne, "addOne")
