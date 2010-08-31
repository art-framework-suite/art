#ifndef DataFormats_Provenance_RunAux_h
#define DataFormats_Provenance_RunAux_h

#include <iosfwd>

#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/RunID.h"

// Auxiliary run information that is persistent.
// Obsolete format, used for backward compatibility only.

namespace edm
{
  struct RunAuxiliary;
  struct RunAux {
    RunAux() : processHistoryID_(), id_() {}
    ~RunAux() {}
    mutable ProcessHistoryID processHistoryID_;
    RunNumber_t id_;
  };
  void conversion(RunAux const& from, RunAuxiliary & to);
}
#endif
