#ifndef art_Persistency_Provenance_ModuleContext_h
#define art_Persistency_Provenance_ModuleContext_h

#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Persistency/Provenance/PathContext.h"

namespace art {
  class ModuleContext {
    explicit ModuleContext() = default;

  public:
    explicit ModuleContext(PathContext const& pathContext,
                           ModuleDescription const& md)
      : pathContext_{pathContext}, md_{md}
    {}

    // This constructor is used in cases where the path context is
    // unneeded.
    explicit ModuleContext(ModuleDescription const& md) : md_{md} {}

    static ModuleContext
    invalid()
    {
      return ModuleContext{};
    }

    auto
    scheduleID() const
    {
      return pathContext_.scheduleID();
    }
    auto const&
    pathName() const
    {
      return pathContext_.pathName();
    }
    auto const&
    moduleDescription() const
    {
      return md_;
    }
    auto const&
    moduleLabel() const
    {
      return md_.moduleLabel();
    }
    auto const&
    moduleName() const
    {
      return md_.moduleName();
    }
    bool
    onEndPath() const
    {
      return pathName() == PathContext::end_path();
    }
    bool
    onTriggerPath() const
    {
      return pathName() != PathContext::end_path() &&
             pathName() != PathContext::art_path() && !pathName().empty();
    }
    bool
    onSamePathAs(std::string const& module_label) const
    {
      return pathContext_.contains(module_label);
    }

  private:
    PathContext pathContext_{PathContext::invalid()};
    ModuleDescription md_{};
  };
}

#endif /* art_Persistency_Provenance_ModuleContext_h */

// Local Variables:
// mode: c++
// End:
