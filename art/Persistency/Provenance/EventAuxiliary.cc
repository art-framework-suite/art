#include "art/Persistency/Provenance/EventAuxiliary.h"
#include <ostream>

void
art::EventAuxiliary::write(std::ostream& os) const {
   os << "Process History ID = " <<  processHistoryID_ << std::endl;
   os << id_ << std::endl;
}

bool
art::isSameEvent(EventAuxiliary const &left, EventAuxiliary const &right) {
   return
      left.id_ == right.id_ &&
      left.processGUID_ == right.processGUID_ &&
      left.time_ == right.time_ &&
      left.isRealData_ == right.isRealData_ &&
      left.experimentType_ == right.experimentType_ &&
      left.bunchCrossing_ == right.bunchCrossing_ &&
      left.storeNumber_ == right.storeNumber_;
}
