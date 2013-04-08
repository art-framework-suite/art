#include "art/Framework/Core/ModuleMacros.h"
#include "test/Integration/GenericOneSimpleProductAnalyzer.h"
#include "test/TestObjects/ToyProducts.h"

namespace arttest {
   typedef GenericOneSimpleProductAnalyzer<int, CompressedIntProduct>
      CompressedIntTestAnalyzer;
}

DEFINE_ART_MODULE(arttest::CompressedIntTestAnalyzer)
