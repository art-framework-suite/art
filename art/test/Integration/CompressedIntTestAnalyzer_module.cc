#include "art/Framework/Core/ModuleMacros.h"
#include "art/test/Integration/GenericOneSimpleProductAnalyzer.h"
#include "art/test/TestObjects/ToyProducts.h"

namespace arttest {
  using CompressedIntTestAnalyzer =
    GenericOneSimpleProductAnalyzer<int, CompressedIntProduct>;
}

DEFINE_ART_MODULE(arttest::CompressedIntTestAnalyzer)
