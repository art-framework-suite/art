#include "art/Utilities/ToolConfigTable.h"
#include "art/Utilities/ToolMacros.h"
#include "art/test/Utilities/tools/OperationBase.h"
#include "fhiclcpp/types/Atom.h"

namespace {

  struct Config {
    fhicl::Atom<int> incrementBy{fhicl::Name("incrementBy"), 1};
  };

  class AddNumber : public arttest::OperationBase {
  public:
    using Parameters = art::ToolConfigTable<Config>;
    explicit AddNumber(Parameters const& config)
      : incrementBy_{config().incrementBy()}
    {}

  private:
    int incrementBy_;

    void
    do_adjustNumber(int& i) const override
    {
      i += incrementBy_;
    }
  };

} // namespace

DEFINE_ART_CLASS_TOOL(AddNumber)
