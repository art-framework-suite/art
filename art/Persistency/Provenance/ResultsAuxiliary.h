#ifndef art_Persistency_Provenance_ResultsAuxiliary_h
#define art_Persistency_Provenance_ResultsAuxiliary_h

#include <iosfwd>
#include <set>

#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/Timestamp.h"

// Auxiliary run data that is persistent

namespace art {
   class ResultsAuxiliary;
}

class art::ResultsAuxiliary {
public:
   ResultsAuxiliary()
      :
      processHistoryID_(),
      allEventsProcessHistories_()
   {}

   void write(std::ostream& os) const;

   ProcessHistoryID& processHistoryID() const { return processHistoryID_; }

   void setProcessHistoryID(ProcessHistoryID const &phid) const
   { processHistoryID_ = phid; }

   bool mergeAuxiliary(ResultsAuxiliary const &aux);

   // most recent process that put a RunProduct into this run
   // is the last on the list, this defines what "latest" is
   mutable ProcessHistoryID processHistoryID_;

   // allEventsProcessHistories_ contains all the ProcessHistoryIDs for all
   // events in this run seen so far.
   std::set<ProcessHistoryID> allEventsProcessHistories_;
};

inline
std::ostream&
operator<<(std::ostream& os, const art::ResultsAuxiliary &p) {
   p.write(os);
   return os;
}

#endif /* art_Persistency_Provenance_ResultsAuxiliary_h */

// Local Variables:
// mode: c++
// End:
