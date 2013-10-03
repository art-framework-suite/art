#ifndef art_Persistency_Provenance_ExecutionContext_h
#define art_Persistency_Provenance_ExecutionContext_h

#include "art/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/exempt_ptr.h"

#include <string>

namespace art {
  class ExecutionContext;

  bool compareContexts(ExecutionContext const & left,
                       ExecutionContext const & right);
  bool sortContexts(ExecutionContext const & left,
                    ExecutionContext const & right);
}

class art::ExecutionContext {
public:
  enum class ExecutionEnvironment {
    INVALID,
      MAIN,
      SCHEDULE,
      END_PATH,
      ON_DEMAND
      };

  ExecutionContext(ExecutionEnvironment env,
              std::string const & stage,
              ModuleDescription const & md);

  ExecutionContext(ExecutionEnvironment env,
              std::string const & stage);

  ExecutionContext(ExecutionEnvironment env,
              std::string && stage,
              ModuleDescription const & md);

  ExecutionContext(ExecutionEnvironment env,
              std::string && stage);

  ExecutionContext(ExecutionContext const & model,
              ModuleDescription const & desc);

  explicit operator bool () const; // True if !isNull();

  bool isNull() const;

  ExecutionEnvironment environment() const;
  std::string const & stage() const;
  cet::exempt_ptr<ModuleDescription const> moduleDescription() const;
  // Original not-on-demand context.
  cet::exempt_ptr<ExecutionContext const> originatingContext() const;
  // Originating context, or current context if there isn't one.
  ExecutionContext const & baseContext() const;

private:
  cet::exempt_ptr<ExecutionContext const> findOriginatingContext_() const;

  ExecutionEnvironment env_;
  std::string stage_;
  cet::exempt_ptr<ModuleDescription const> md_;
  cet::exempt_ptr<ExecutionContext const> oc_;
 };

inline
art::ExecutionContext::
ExecutionContext(ExecutionEnvironment env,
            std::string const & stage,
            ModuleDescription const & md)
:
  env_(env),
  stage_(stage),
  md_(&md),
  oc_(findOriginatingContext_())
{
}

inline
art::ExecutionContext::
ExecutionContext(ExecutionEnvironment env,
            std::string const & stage)
:
  env_(env),
  stage_(stage),
  md_(nullptr),
  oc_(findOriginatingContext_())
{
}

inline
art::ExecutionContext::
ExecutionContext(ExecutionEnvironment env,
            std::string && stage,
            ModuleDescription const & md)
:
  env_(env),
  stage_(std::move(stage)),
  md_(&md),
  oc_(findOriginatingContext_())
{
}

inline
art::ExecutionContext::
ExecutionContext(ExecutionEnvironment env,
            std::string && stage)
:
  env_(env),
  stage_(std::move(stage)),
  md_(nullptr),
  oc_(findOriginatingContext_())
{
}

inline
art::ExecutionContext::
ExecutionContext(ExecutionContext const & model,
            ModuleDescription const & desc)
:
  ExecutionContext(model.env_, model.stage_, desc)
{
}

inline
art::ExecutionContext::
operator bool() const
{
  return !isNull();
}

inline
bool
art::ExecutionContext::
isNull() const
{
  return env_ == ExecutionEnvironment::INVALID;
}

inline
auto
art::ExecutionContext::
environment() const
-> ExecutionEnvironment
{
  return env_;
}

inline
std::string const &
art::ExecutionContext::
stage() const
{
  return stage_;
}

inline
auto
art::ExecutionContext::
moduleDescription() const
-> cet::exempt_ptr<ModuleDescription const>
{
  return md_;
}

inline
auto
art::ExecutionContext::
originatingContext() const
-> cet::exempt_ptr<ExecutionContext const>
{
  return oc_;
}

inline
auto
art::ExecutionContext::
baseContext() const
-> ExecutionContext const &
{
  return oc_ ? *oc_ : *this;
}

inline
bool
art::compareContexts(ExecutionContext const & left,
                     ExecutionContext const & right)
{
  auto const & bl = left.baseContext();
  auto const & br = right.baseContext();
  return bl.environment() == br.environment() ||
    bl.stage() == br.stage() ||
    (bl.environment() !=
     art::ExecutionContext::ExecutionEnvironment::END_PATH ||
     bl.moduleDescription().get() == br.moduleDescription().get());
}

inline
bool
art::sortContexts(ExecutionContext const & left,
                  ExecutionContext const & right)
{
  auto const & bl = left.baseContext();
  auto const & br = right.baseContext();
  // Attempt to have strict weak ordering from something which is more
  // naturally a simple yes/no answer.
  return bl.environment() < br.environment() ||
    bl.stage() < br.stage() ||
    (bl.environment() ==
     art::ExecutionContext::ExecutionEnvironment::END_PATH &&
     bl.moduleDescription().get() < br.moduleDescription().get());
}

#endif /* art_Persistency_Provenance_ExecutionContext_h */

// Local Variables:
// mode: c++
// End:
