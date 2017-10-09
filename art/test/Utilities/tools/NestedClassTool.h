#ifndef art_test_Utilities_tools_NestedClassTool_h
#define art_test_Utilities_tools_NestedClassTool_h

#include "art/Utilities/make_tool.h"
#include "art/test/Utilities/tools/ClassTool.h"
#include "fhiclcpp/ParameterSet.h"

namespace arttest {
  class NestedClassTool {
  public:
    explicit NestedClassTool(fhicl::ParameterSet const& ps) : pset_{ps} {}

    int
    callThroughToAddOne(int const i)
    {
      auto t = art::make_tool<ClassTool>(pset_);
      return t->addOne(i);
    }

  private:
    fhicl::ParameterSet pset_;
  };
}

#endif /* art_test_Utilities_tools_NestedClassTool_h */

// Local variables:
// mode: c++
// End:
