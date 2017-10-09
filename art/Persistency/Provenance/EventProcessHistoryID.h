#ifndef art_Persistency_Provenance_EventProcessHistoryID_h
#define art_Persistency_Provenance_EventProcessHistoryID_h

#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/ProcessHistoryID.h"

namespace art {
  struct EventProcessHistoryID {
    EventProcessHistoryID() : eventID_(), processHistoryID_() {}
    EventProcessHistoryID(EventID const& id, ProcessHistoryID const& ph)
      : eventID_(id), processHistoryID_(ph)
    {}
    EventID eventID_;
    ProcessHistoryID processHistoryID_;
  };
  inline bool
  operator<(EventProcessHistoryID const& lh, EventProcessHistoryID const& rh)
  {
    return lh.eventID_ < rh.eventID_;
  }
}

#endif /* art_Persistency_Provenance_EventProcessHistoryID_h */

// Local Variables:
// mode: c++
// End:
