#ifndef art_Persistency_Provenance_ExecEnvInfo_h
#define art_Persistency_Provenance_ExecEnvInfo_h

#include "art/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/exempt_ptr.h"

#include <string>

namespace art {
  class ExecEnvInfo;
}

class art::ExecEnvInfo {
public:
  enum class ExecutionEnvironment {
    MAIN,
      SCHEDULE,
      END_PATH,
      ON_DEMAND
      };

  ExecEnvInfo(ExecutionEnvironment env,
              std::string const & stage,
              ModuleDescription const & md);

  ExecEnvInfo(ExecutionEnvironment env,
              std::string const & stage);

  ExecEnvInfo(ExecutionEnvironment env,
              std::string && stage,
              ModuleDescription const & md);

  ExecEnvInfo(ExecutionEnvironment env,
              std::string && stage);

  ExecEnvInfo(ExecEnvInfo const & model,
              ModuleDescription const & desc);

  ExecutionEnvironment environment() const;
  std::string const & stage() const;
  cet::exempt_ptr<ModuleDescription const> moduleDescription() const;
  // Original not-on-demand context.
  cet::exempt_ptr<ExecEnvInfo const> originatingContext() const;
  // Originating context, or current context if there isn't one.
  ExecEnvInfo const & baseContext() const;

private:
  cet::exempt_ptr<ExecEnvInfo const> findOriginatingContext_() const;

  ExecutionEnvironment env_;
  std::string stage_;
  cet::exempt_ptr<ModuleDescription const> md_;
  cet::exempt_ptr<ExecEnvInfo const> oc_;
 };

inline
art::ExecEnvInfo::
ExecEnvInfo(ExecutionEnvironment env,
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
art::ExecEnvInfo::
ExecEnvInfo(ExecutionEnvironment env,
            std::string const & stage)
:
  env_(env),
  stage_(stage),
  md_(nullptr),
  oc_(findOriginatingContext_())
{
}

inline
art::ExecEnvInfo::
ExecEnvInfo(ExecutionEnvironment env,
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
art::ExecEnvInfo::
ExecEnvInfo(ExecutionEnvironment env,
            std::string && stage)
:
  env_(env),
  stage_(std::move(stage)),
  md_(nullptr),
  oc_(findOriginatingContext_())
{
}

inline
art::ExecEnvInfo::
ExecEnvInfo(ExecEnvInfo const & model,
            ModuleDescription const & desc)
:
  ExecEnvInfo(model.env_, model.stage_, desc)
{
}

inline
auto
art::ExecEnvInfo::
environment() const
-> ExecutionEnvironment
{
  return env_;
}

inline
std::string const &
art::ExecEnvInfo::
stage() const
{
  return stage_;
}

inline
auto
art::ExecEnvInfo::
moduleDescription() const
-> cet::exempt_ptr<ModuleDescription const>
{
  return md_;
}

inline
auto
art::ExecEnvInfo::
originatingContext() const
-> cet::exempt_ptr<ExecEnvInfo const>
{
  return oc_;
}

inline
auto
art::ExecEnvInfo::
baseContext() const
-> ExecEnvInfo const &
{
  return oc_ ? *oc_ : *this;
}
#endif /* art_Persistency_Provenance_ExecEnvInfo_h */

// Local Variables:
// mode: c++
// End:
