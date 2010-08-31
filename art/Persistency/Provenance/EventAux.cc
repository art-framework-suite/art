#include "art/Persistency/Provenance/EventAux.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"

/*----------------------------------------------------------------------



----------------------------------------------------------------------*/

namespace edm {
  void conversion(EventAux const& from, EventAuxiliary & to) {
    to.processHistoryID_ = from.processHistoryID_;
    to.id_ = from.id_;
    to.time_ = from.time_;
    to.luminosityBlock_ = from.luminosityBlockID_;
  }
}
