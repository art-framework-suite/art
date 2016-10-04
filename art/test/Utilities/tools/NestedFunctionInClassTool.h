#ifndef art_test_Utilities_NestedFunctionInClassTool_h
#define art_test_Utilities_NestedFunctionInClassTool_h

#include "art/Utilities/make_tool.h"
#include "fhiclcpp/ParameterSet.h"

namespace arttest {
  class NestedFunctionInClassTool {
  public:

    explicit NestedFunctionInClassTool(fhicl::ParameterSet const& ps) : pset_{ps} {}

    int callThroughToAddOne(int const i)
    {
      auto addOne = art::make_tool<int(int)>(pset_.get<fhicl::ParameterSet>("addOneTool"));
      return addOne(i);
    }

  private:
    fhicl::ParameterSet pset_;
  };
}

#endif

// Local variables:
// mode: c++
// End:
