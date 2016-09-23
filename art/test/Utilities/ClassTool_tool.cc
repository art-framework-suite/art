#include "art/Framework/Core/ToolMacros.h"
#include "art/test/Utilities/ClassTool.h"

#include <iostream>

arttest::ClassTool::ClassTool(fhicl::ParameterSet const&)
{
  std::cout << " Wow, this worked.\n";
}

DEFINE_ART_TOOL_CLASS(arttest::ClassTool)
