#include "art/Framework/Services/Registry/detail/verifyContext.h"
#include "art/Persistency/Provenance/ExecutionContext.h"
#include "art/Utilities/Exception.h"

bool
art::detail::
verifyContext(ExecEnvInfo const & oc)
{
  ExecEnvInfo const & cc(ExecutionContext::top().baseContext());

  if (oc.environment() == cc.environment() &&
      oc.stage() == cc.stage &&
      (oc.environment() != ExecutionEnvironment::END_PATH ||
       &oc.moduleDescription() == &cc.moduleDescription())) {
    return true;
  }
  return false;
}
