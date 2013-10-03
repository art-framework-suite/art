#include "art/Persistency/Provenance/ExecutionContext.h"
#include "art/Persistency/Provenance/ExecutionContextManager.h"

auto
art::ExecutionContext::
findOriginatingContext_() const
-> cet::exempt_ptr<ExecutionContext const>
{
  cet::exempt_ptr<ExecutionContext const> result;
  if (env_ == ExecutionEnvironment::ON_DEMAND &&
      ! ExecutionContextManager::empty() ) {
    ExecutionContext const & parent(ExecutionContextManager::top());
    if (parent.originatingContext()) {
      result = parent.originatingContext();
    }
    else {
      result.reset(&parent);
    }
  }
  return result;
}
