#include "art/Framework/Core/detail/get_failureToPut_flag.h"
#include "fhiclcpp/ParameterSet.h"

using fhicl::ParameterSet;

namespace art {

  bool
  detail::get_failureToPut_flag(ParameterSet const& scheduler_pset,
                                ParameterSet const& module_pset)
  {
    bool const global_flag {scheduler_pset.get<bool>("errorOnFailureToPut")};
    bool const local_flag {module_pset.get<bool>("errorOnFailureToPut", true)};
    return !global_flag ? global_flag : local_flag; // global flag set to 'false' always wins
  }

}
