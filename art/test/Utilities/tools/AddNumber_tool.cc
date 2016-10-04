#include "art/Framework/Core/ToolMacros.h"
#include "art/test/Utilities/tools/OperationBase.h"

namespace fhicl { class ParameterSet; }

namespace arttest {
  class AddNumber : public OperationBase {
  public:
    explicit AddNumber(fhicl::ParameterSet const&){}
  private:
    void do_adjustNumber(int& i) const override { ++i; }
  };
}

DEFINE_ART_TOOL_CLASS(arttest::AddNumber)
