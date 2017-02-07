#include "art/Framework/Core/ModuleMacros.h"
#include "art/test/Integration/GenericOneSimpleProductAnalyzer.h"
#include "art/test/TestObjects/ToyProducts.h"

namespace arttest {
  typedef GenericOneSimpleProductAnalyzer<int, IntProduct> IntTestAnalyzer;
}

DEFINE_ART_MODULE(arttest::IntTestAnalyzer)
