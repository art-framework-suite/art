#include "art/Utilities/ToolMacros.h"
#include "art/Utilities/make_tool.h"

namespace arttest {
  int callThroughToAddOne(fhicl::ParameterSet const& pset, int const i)
  {
    auto addOne = art::make_tool<int(int)>(pset);
    return addOne(i);
  }
}

DEFINE_ART_TOOL_FUNCTION(arttest::callThroughToAddOne)
