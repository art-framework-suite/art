#ifndef art_Persistency_Provenance_PathContext_h
#define art_Persistency_Provenance_PathContext_h

#include "art/Persistency/Provenance/ScheduleContext.h"

namespace art {
  class PathContext {
  public:
    static std::string
    end_path()
    {
      return "end_path";
    }

    explicit PathContext(ScheduleContext const& scheduleContext,
                         std::string const& pathName,
                         int const bitPosition)
      : scheduleContext_{scheduleContext}
      , pathName_{pathName}
      , isEndPath_{pathName == end_path()}
      , bitPosition_{bitPosition}
    {}

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
    bool
    isEndPath() const
    {
      return isEndPath_;
    }
    int
    bitPosition() const
    {
      return bitPosition_;
    }

  private:
    ScheduleContext scheduleContext_;
    std::string pathName_;
    bool isEndPath_;
    int bitPosition_;
  };
}

#endif
