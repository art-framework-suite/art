#ifndef art_test_Utilities_tools_ClassTool_h
#define art_test_Utilities_tools_ClassTool_h

namespace fhicl {
  class ParameterSet;
}

namespace arttest {
  struct ClassTool {
    ClassTool(fhicl::ParameterSet const&) {}
    int
    addOne(int const i)
    {
      return i + 1;
    }
  };
} // namespace arttest

#endif /* art_test_Utilities_tools_ClassTool_h */

// Local variables:
// mode: c++
// End:
