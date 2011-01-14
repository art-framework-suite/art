#include "art/Framework/Core/ModuleMacros.h"
#include "test/Integration/GenericOneSimpleProductAnalyzer.h"
#include "test/TestObjects/ToyProducts.h"

namespace arttest {
   typedef GenericOneSimpleProductAnalyzer<double, DoubleProduct> DoubleTestAnalyzer;
}

DEFINE_ART_MODULE(arttest::DoubleTestAnalyzer);
