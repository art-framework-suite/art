#include "art/Framework/Core/ModuleMacros.h"
#include "art/test/Integration/GenericOneSimpleProductAnalyzer.h"

#include <string>

namespace arttest {
  typedef GenericOneSimpleProductAnalyzer<std::string, std::string> BareStringAnalyzer;
}

DEFINE_ART_MODULE(arttest::BareStringAnalyzer)
