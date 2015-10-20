#include "art/Framework/Core/detail/get_failureToPut_flag.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"

namespace art {

  bool
  detail::get_failureToPut_flag(art::ModuleDescription const& md) {
    auto const& main_pset   = fhicl::ParameterSetRegistry::get( md.mainParameterSetID() );
    auto const& pset        = fhicl::ParameterSetRegistry::get( md.parameterSetID() );
    bool const  global_flag = main_pset.get<bool>("services.scheduler.errorOnFailureToPut");
    bool const  local_flag  = pset.get<bool>("errorOnFailureToPut", true);
    return !global_flag ? global_flag : local_flag; // global flag set to 'false' always wins
  }

}
