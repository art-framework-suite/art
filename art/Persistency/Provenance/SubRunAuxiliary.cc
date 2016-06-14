#include "art/Persistency/Provenance/SubRunAuxiliary.h"
#include <ostream>

void
art::SubRunAuxiliary::write(std::ostream& os) const {
   os << "Process History ID = " <<  processHistoryID_ << std::endl;
   os << id_ << std::endl;
}

bool
art::SubRunAuxiliary::mergeAuxiliary(SubRunAuxiliary const &newAux) {
   if (beginTime_ == Timestamp::invalidTimestamp() ||
       newAux.beginTime() == Timestamp::invalidTimestamp()) {
      beginTime_ = Timestamp::invalidTimestamp();
   }
   else if (newAux.beginTime() < beginTime_) {
      beginTime_ = newAux.beginTime();
   }

   if (endTime_ == Timestamp::invalidTimestamp() ||
       newAux.endTime() == Timestamp::invalidTimestamp()) {
      endTime_ = Timestamp::invalidTimestamp();
   }
   else if (newAux.endTime() > endTime_) {
      endTime_ = newAux.endTime();
   }

   // Keep the process history ID that is in the preexisting principal
   // It may have been updated to include the current process.
   // There is one strange other case where the two ProcessHistoryIDs
   // could be different which should not be important and we just ignore.
   // There could have been previous processes which only dropped products.
   // These processes could have dropped the same branches but had different
   // process names ... Ignore this.

   if (id_ != newAux.id()) return false;
   return true;
}
