#ifndef art_Persistency_Provenance_ExecutionContext_h
#define art_Persistency_Provenance_ExecutionContext_h

#include "art/Persistency/Provenance/ExecEnvInfo.h"
#include "art/Persistency/Provenance/ModuleDescription.h"

#include <stack>
#include <thread>

namespace art {
  // This could be a namespace, except for the fact we want to protect
  // access to contextStack_.
  class ExecutionContext;
}

class art::ExecutionContext {
public:
  static void push(ExecEnvInfo const & item);
  static void push(ExecEnvInfo && item);
  static void push(ModuleDescription const & desc); // Based on current context.

  template <typename ... Args>
  static void emplace(Args && ... args);

  static bool empty();
  static ExecEnvInfo const & top();
  static void pop();

private:
  ExecutionContext() = delete; // No objects.
  static thread_local std::stack<ExecEnvInfo> contextStack_;
};

inline
void
art::ExecutionContext::
push(ExecEnvInfo const & item)
{
  contextStack_.push(item);
}

inline
void
art::ExecutionContext::
push(ExecEnvInfo && item)
{
  contextStack_.push(std::move(item));
}

inline
void
art::ExecutionContext::
push(art::ModuleDescription const & desc)
{
  push(ExecEnvInfo(top(), desc));
}

inline
bool
art::ExecutionContext::
empty()
{
  return contextStack_.empty();
}

inline
art::ExecEnvInfo const &
art::ExecutionContext::
top()
{
  return contextStack_.top();
}

inline
void
art::ExecutionContext::
pop()
{
  contextStack_.pop();
}

template <typename ... Args>
inline
void
art::ExecutionContext::
emplace(Args && ... args)
{
  contextStack_.emplace(std::forward<Args...>(args...));
}

#endif /* art_Persistency_Provenance_ExecutionContext_h */

// Local Variables:
// mode: c++
// End:
