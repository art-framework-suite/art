#ifndef art_Persistency_Provenance_PathContext_h
#define art_Persistency_Provenance_PathContext_h

#include "art/Persistency/Provenance/PathSpec.h"
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "cetlib/container_algorithms.h"

namespace art {
  class PathContext {
    explicit PathContext() = default;

  public:
    static std::string
    end_path()
    {
      return "end_path";
    }

    static auto
    end_path_spec()
    {
      return PathSpec{end_path(), PathID{0}};
    }

    static std::string
    art_path()
    {
      return "[art]";
    }

    static auto
    art_path_spec()
    {
      return PathSpec{art_path(), PathID::invalid()};
    }

    explicit PathContext(ScheduleContext const& scheduleContext,
                         PathSpec const& pathSpec,
                         std::vector<std::string> sortedModuleNames)
      : scheduleContext_{scheduleContext}
      , pathSpec_{pathSpec}
      , sortedModuleLabels_{std::move(sortedModuleNames)}
    {}

    static PathContext
    invalid()
    {
      return PathContext{};
    }

    auto
    scheduleID() const
    {
      return scheduleContext_.id();
    }
    auto const&
    pathSpec() const
    {
      return pathSpec_;
    }

    auto const&
    pathName() const
    {
      return pathSpec_.name;
    }
    PathID
    pathID() const noexcept
    {
      return pathSpec_.path_id;
    }

    bool
    contains(std::string const& module_label) const
    {
      return cet::binary_search_all(sortedModuleLabels_, module_label);
    }

  private:
    ScheduleContext scheduleContext_{ScheduleContext::invalid()};
    PathSpec pathSpec_{"", PathID::invalid()};
    std::vector<std::string> sortedModuleLabels_{};
  };
}

#endif /* art_Persistency_Provenance_PathContext_h */

// Local Variables:
// mode: c++
// End:
