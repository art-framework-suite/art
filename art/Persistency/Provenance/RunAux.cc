#include "art/Persistency/Provenance/RunAux.h"
#include "art/Persistency/Provenance/RunAuxiliary.h"
#include "art/Persistency/Provenance/Timestamp.h"

/*----------------------------------------------------------------------



----------------------------------------------------------------------*/

namespace art {
  void conversion(RunAux const& from, RunAuxiliary & to) {
    to.processHistoryID_ = from.processHistoryID_;
    to.id_ = RunID(from.id_);
    to.beginTime_ = to.endTime_ = Timestamp::invalidTimestamp();
  }
}
