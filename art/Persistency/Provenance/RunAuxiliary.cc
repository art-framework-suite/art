#include "art/Persistency/Provenance/RunAuxiliary.h"
#include <ostream>

void
art::RunAuxiliary::write(std::ostream& os) const {
   os << "Process History ID = " <<  processHistoryID_ << std::endl;
   os << id_ << std::endl;
}

bool
art::RunAuxiliary::mergeAuxiliary(RunAuxiliary const &newAux) {

   mergeNewTimestampsIntoThis_(newAux);
   mergeNewProcessHistoryIntoThis_(newAux);

   // Keep the process history ID that is in the preexisting principal
   // It may have been updated to include the current process.
   // There is one strange other case where the two ProcessHistoryIDs
   // could be different which should not be important and we just ignore.
   // There could have been previous processes which only dropped products.
   // These processes could have dropped the same branches but had different
   // process names ... Ignore this.

   return id_ == newAux.id();
   //     if (id_ != newAux.id()) return false;
   //     return true;
}

void
art::RunAuxiliary::mergeNewTimestampsIntoThis_(RunAuxiliary const &newAux) {
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
}

void
art::RunAuxiliary::mergeNewProcessHistoryIntoThis_(RunAuxiliary const &newAux) {
   allEventsProcessHistories_.insert(newAux.allEventsProcessHistories_.begin(),
                                     newAux.allEventsProcessHistories_.end());
}
