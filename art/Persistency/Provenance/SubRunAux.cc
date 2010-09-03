#include "art/Persistency/Provenance/SubRunAux.h"
#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include "art/Persistency/Provenance/Timestamp.h"

/*----------------------------------------------------------------------



----------------------------------------------------------------------*/

namespace edm {
  void conversion(SubRunAux const& from, SubRunAuxiliary & to) {
    to.processHistoryID_ = from.processHistoryID_;
    to.id_ = SubRunID(from.runID_, from.id_);
    to.beginTime_ = to.endTime_ = Timestamp::invalidTimestamp();
  }
}
