#include "art/Utilities/ToolMacros.h"
#include "art/Utilities/make_tool.h"
#include "art/test/Utilities/tools/NestedFunctionTool.h"

namespace arttest {
  int
  callThroughToAddOne(fhicl::ParameterSet const& pset, int const i)
  {
    auto addOne = art::make_tool<int(int)>(pset, "addOne");
    return addOne(i);
  }
} // namespace arttest

DEFINE_ART_FUNCTION_TOOL(arttest::callThroughToAddOne, "callThroughToAddOne")
