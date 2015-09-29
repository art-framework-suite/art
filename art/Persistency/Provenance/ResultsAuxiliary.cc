#include "art/Persistency/Provenance/ResultsAuxiliary.h"
#include <ostream>

void
art::ResultsAuxiliary::write(std::ostream& os) const {
   os << "Process History ID = " <<  processHistoryID_ << std::endl;
}

bool
art::ResultsAuxiliary::mergeAuxiliary(ResultsAuxiliary const &newAux) {

   allEventsProcessHistories_.insert(newAux.allEventsProcessHistories_.begin(),
                                     newAux.allEventsProcessHistories_.end());

   // Keep the process history ID that is in the preexisting principal
   // It may have been updated to include the current process.
   // There is one strange other case where the two ProcessHistoryIDs
   // could be different which should not be important and we just ignore.
   // There could have been previous processes which only dropped products.
   // These processes could have dropped the same branches but had different
   // process names ... Ignore this.

   return true;
}
