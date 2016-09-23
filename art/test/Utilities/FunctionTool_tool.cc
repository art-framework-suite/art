#include "art/Framework/Core/ToolMacros.h"

#include <iostream>

namespace arttest {
  int print(int const i)
  {
    std::cout << " Wow, this worked: " << i << ".\n";
    return i;
  }
}

DEFINE_ART_TOOL_FUNCTION(arttest::print)
