#include "art/Utilities/ToolMacros.h"
#include "art/test/Utilities/tools/OperationBase.h"

namespace fhicl { class ParameterSet; }

namespace arttest {
  class SubtractNumber : public OperationBase {
  public:
    explicit SubtractNumber(fhicl::ParameterSet const&) {}
  private:
    void do_adjustNumber(int& i) const override { --i; }
  };
}

DEFINE_ART_TOOL_CLASS(arttest::SubtractNumber)
