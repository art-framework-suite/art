#include "art/Persistency/Provenance/LuminosityBlockAux.h"
#include "art/Persistency/Provenance/LuminosityBlockAuxiliary.h"
#include "art/Persistency/Provenance/Timestamp.h"

/*----------------------------------------------------------------------



----------------------------------------------------------------------*/

namespace edm {
  void conversion(LuminosityBlockAux const& from, LuminosityBlockAuxiliary & to) {
    to.processHistoryID_ = from.processHistoryID_;
    to.id_ = LuminosityBlockID(from.runID_, from.id_);
    to.beginTime_ = to.endTime_ = Timestamp::invalidTimestamp();
  }
}
