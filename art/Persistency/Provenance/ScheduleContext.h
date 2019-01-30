#ifndef art_Persistency_Provenance_ScheduleContext_h
#define art_Persistency_Provenance_ScheduleContext_h

#include "art/Utilities/ScheduleID.h"

namespace art {
  class ScheduleContext {
    explicit ScheduleContext() = default;

  public:
    explicit ScheduleContext(ScheduleID const sid) : sid_{sid} {}
    auto
    id() const
    {
      return sid_;
    }

    static ScheduleContext
    invalid()
    {
      return ScheduleContext{};
    }

  private:
    ScheduleID sid_{};
  };
}

#endif /* art_Persistency_Provenance_ScheduleContext_h */

// Local Variables:
// mode: c++
// End:
