#include "art/Utilities/ToolMacros.h"

namespace {
  void addInts(int& i, int const j) { i += j; }
}

DEFINE_ART_TOOL_FUNCTION(addInts)
