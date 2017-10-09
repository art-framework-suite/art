#ifndef art_test_Framework_Art_PrintAvailable_DummyClassTool_h
#define art_test_Framework_Art_PrintAvailable_DummyClassTool_h

#include "art/Utilities/ToolConfigTable.h"

namespace fhicl {
  class ParameterSet;
}

namespace art {
  namespace test {
    struct DummyClassTool {
      struct Config {
      };
      using Parameters = ToolConfigTable<Config>;
      DummyClassTool(Parameters const&) {}
    };
  }
}

#endif /* art_test_Framework_Art_PrintAvailable_DummyClassTool_h */

// Local variables:
// mode: c++
// End:
