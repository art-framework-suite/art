#include "art/Utilities/ToolMacros.h"

namespace arttest {
  int
  addOne(int const i)
  {
    return i + 1;
  }
}

DEFINE_ART_FUNCTION_TOOL(arttest::addOne, "addOne")
