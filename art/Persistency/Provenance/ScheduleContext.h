#ifndef art_Persistency_Provenance_ScheduleContext_h
#define art_Persistency_Provenance_ScheduleContext_h

#include "art/Utilities/ScheduleID.h"

namespace art {
  class ScheduleContext {
  public:
    explicit ScheduleContext(ScheduleID const sid) : sid_{sid} {}
    auto
    id() const
    {
      return sid_;
    }

  private:
    ScheduleID sid_;
  };
}

#endif /* art_Persistency_Provenance_ScheduleContext_h */

// Local Variables:
// mode: c++
// End:
