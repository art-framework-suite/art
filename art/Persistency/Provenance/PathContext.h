#ifndef art_Persistency_Provenance_PathContext_h
#define art_Persistency_Provenance_PathContext_h

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

    static std::string
    art_path()
    {
      return "[art]";
    }

    explicit PathContext(ScheduleContext const& scheduleContext,
                         std::string const& pathName,
                         int const bitPosition,
                         std::vector<std::string> sortedModuleNames)
      : scheduleContext_{scheduleContext}
      , pathName_{pathName}
      , bitPosition_{bitPosition}
      , sortedModuleLabels_{move(sortedModuleNames)}
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
    pathName() const
    {
      return pathName_;
    }
    int
    bitPosition() const
    {
      return bitPosition_;
    }

    bool
    contains(std::string const& module_label) const
    {
      return cet::binary_search_all(sortedModuleLabels_, module_label);
    }

  private:
    ScheduleContext scheduleContext_{ScheduleContext::invalid()};
    std::string pathName_{};
    int bitPosition_{-1};
    std::vector<std::string> sortedModuleLabels_{};
  };
}

#endif /* art_Persistency_Provenance_PathContext_h */

// Local Variables:
// mode: c++
// End:
