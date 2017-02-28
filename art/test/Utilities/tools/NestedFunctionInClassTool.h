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
      // It is a bad idea to call make_tool for each function call, as
      // is the case here.  It would be much better to create a data
      // member of type "std::function<int(int)>" that is initialized
      // in the c'tor.  However, to demonstrate that a tool CAN be
      // constructed within a function call, we leave it as is.  See
      // the c'tor of AddIntsProducer for an example of better code
      // practice.
      auto addOne = art::make_tool<int(int)>(pset_.get<fhicl::ParameterSet>("addOneTool"), "addOne");
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
