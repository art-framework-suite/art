#include "art/Framework/Core/ModuleMacros.h"
#include "test/Integration/GenericOneSimpleProductAnalyzer.h"
#include "test/TestObjects/ToyProducts.h"

namespace arttest {
   typedef GenericOneSimpleProductAnalyzer<int, IntProduct> IntTestAnalyzer;
}

DEFINE_ART_MODULE(arttest::IntTestAnalyzer);
