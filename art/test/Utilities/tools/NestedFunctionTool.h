#ifndef art_test_Utilities_tools_NestedFunctionTool_h
#define art_test_Utilities_tools_NestedFunctionTool_h

namespace fhicl {
  class ParameterSet;
}

namespace arttest {
  int callThroughToAddOne(fhicl::ParameterSet const& pset, int);
}

// Local variables:
// mode: c++
// End:
#endif /* art_test_Utilities_tools_NestedFunctionTool_h */
