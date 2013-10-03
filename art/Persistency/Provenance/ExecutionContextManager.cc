#include "art/Persistency/Provenance/ExecutionContextManager.h"

thread_local std::stack<art::ExecutionContext>
art::ExecutionContextManager::contextStack_ { };
