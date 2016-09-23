#ifndef art_test_Utilities_ClassTool_h
#define art_test_Utilities_ClassTool_h

namespace fhicl {
  class ParameterSet;
}

namespace arttest {
  struct ClassTool {
    ClassTool(fhicl::ParameterSet const&);
  };
}

#endif

// Local variables:
// mode: c++
// End:
