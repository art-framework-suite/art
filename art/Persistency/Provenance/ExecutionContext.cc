#include "art/Persistency/Provenance/ExecutionContext.h"

thread_local std::stack<art::ExecEnvInfo>
art::ExecutionContext::contextStack_ { };
