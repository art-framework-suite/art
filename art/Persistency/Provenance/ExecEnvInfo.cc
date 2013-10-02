#include "art/Persistency/Provenance/ExecEnvInfo.h"
#include "art/Persistency/Provenance/ExecutionContext.h"

auto
art::ExecEnvInfo::
findOriginatingContext_() const
-> cet::exempt_ptr<ExecEnvInfo const>
{
  cet::exempt_ptr<ExecEnvInfo const> result;
  if (env_ == ExecutionEnvironment::ON_DEMAND &&
      ! ExecutionContext::empty() ) {
    ExecEnvInfo const & parent(ExecutionContext::top());
    if (parent.originatingContext()) {
      result = parent.originatingContext();
    }
    else {
      result.reset(&parent);
    }
  }
  return result;
}
