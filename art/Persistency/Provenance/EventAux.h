#ifndef DataFormats_Provenance_EventAux_h
#define DataFormats_Provenance_EventAux_h

#include <iosfwd>

#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "art/Persistency/Provenance/SubRunID.h"

// Auxiliary event data that is persistent
// Obsolete format, used for backward compatibility only.

namespace art {
  struct EventAuxiliary;
  struct EventAux {
    EventAux() : processHistoryID_(), id_(), time_(), subRunID_() {}
    ~EventAux() {}
    mutable ProcessHistoryID processHistoryID_;
    EventID id_;
    Timestamp time_;
    SubRunNumber_t subRunID_;
  };
  void conversion(EventAux const& from, EventAuxiliary & to);
}
#endif
