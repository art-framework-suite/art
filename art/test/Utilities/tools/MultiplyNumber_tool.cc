#include "art/Framework/Core/ToolMacros.h"
#include "art/test/Utilities/tools/OperationBase.h"

namespace fhicl { class ParameterSet; }

namespace arttest {
  class MultiplyNumber : public OperationBase {
  public:
    explicit MultiplyNumber(fhicl::ParameterSet const&){}
  private:
    void do_adjustNumber(int& i) const override { i *= 2; }
  };
}

DEFINE_ART_TOOL_CLASS(arttest::MultiplyNumber)
