#ifndef art_Persistency_Provenance_ExecutionContextManager_h
#define art_Persistency_Provenance_ExecutionContextManager_h

#include "art/Persistency/Provenance/ExecutionContext.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

#include <stack>
#include <thread>

namespace art {
  // This could be a namespace, except for the fact we want to protect
  // access to contextStack_.
  class ExecutionContextManager;
}

class art::ExecutionContextManager {
public:
  static void push(ExecutionContext const & item);
  static void push(ExecutionContext && item);
  static void push(ModuleDescription const & desc); // Based on current context.

  template <typename ... Args>
  static void emplace(Args && ... args);

  static bool empty();
  static ExecutionContext const & top();
  static void pop();

private:
  ExecutionContextManager() = delete; // No objects.
  static thread_local std::stack<ExecutionContext> contextStack_;
};

inline
void
art::ExecutionContextManager::
push(ExecutionContext const & item)
{
  contextStack_.push(item);
}

inline
void
art::ExecutionContextManager::
push(ExecutionContext && item)
{
  contextStack_.push(std::move(item));
}

inline
void
art::ExecutionContextManager::
push(art::ModuleDescription const & desc)
{
  push(ExecutionContext(top(), desc));
}

inline
bool
art::ExecutionContextManager::
empty()
{
  return contextStack_.empty();
}

inline
art::ExecutionContext const &
art::ExecutionContextManager::
top()
{
  return contextStack_.top();
}

inline
void
art::ExecutionContextManager::
pop()
{
  contextStack_.pop();
}

template <typename ... Args>
inline
void
art::ExecutionContextManager::
emplace(Args && ... args)
{
  contextStack_.emplace(std::forward<Args...>(args...));
}

#endif /* art_Persistency_Provenance_ExecutionContextManager_h */

// Local Variables:
// mode: c++
// End:
