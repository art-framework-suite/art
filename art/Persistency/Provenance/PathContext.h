#ifndef art_Persistency_Provenance_PathContext_h
#define art_Persistency_Provenance_PathContext_h

#include "art/Persistency/Provenance/ScheduleContext.h"

namespace art {
  class PathContext {
  public:
    explicit PathContext(ScheduleContext const& scheduleContext,
                         std::string const& pathName,
                         bool const isEndPath)
      : scheduleContext_{scheduleContext}
      , pathName_{pathName}
      , isEndPath_{isEndPath}
    {}

    auto scheduleID() const { return scheduleContext_.id(); }
    auto const& pathName() const { return pathName_; }
    bool isEndPath() const { return isEndPath_; }

  private:
    ScheduleContext const& scheduleContext_;
    std::string const pathName_;
    bool const isEndPath_;
  };
}

#endif
