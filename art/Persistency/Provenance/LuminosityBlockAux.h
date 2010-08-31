#ifndef DataFormats_Provenance_LuminosityBlockAux_h
#define DataFormats_Provenance_LuminosityBlockAux_h

#include <iosfwd>

#include "art/Persistency/Provenance/LuminosityBlockID.h"
#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/RunID.h"

// Auxiliary luminosity block data that is persistent
namespace edm
{
  struct LuminosityBlockAuxiliary;
  struct LuminosityBlockAux {
    LuminosityBlockAux() : processHistoryID_(), id_(), runID_() {}
    ~LuminosityBlockAux() {}
    mutable ProcessHistoryID processHistoryID_;
    LuminosityBlockNumber_t id_;
    RunNumber_t runID_;
  };
  void conversion(LuminosityBlockAux const& from, LuminosityBlockAuxiliary & to);
}
#endif
