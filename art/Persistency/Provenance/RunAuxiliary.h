#ifndef art_Persistency_Provenance_RunAuxiliary_h
#define art_Persistency_Provenance_RunAuxiliary_h

#include <iosfwd>
#include <set>

#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/Timestamp.h"

// Auxiliary run data that is persistent

namespace art {
   class RunAuxiliary;
}

class art::RunAuxiliary {
public:
   RunAuxiliary()
      :
      processHistoryID_(),
      allEventsProcessHistories_(),
      id_(),
      beginTime_(),
      endTime_()
   {}

   RunAuxiliary(RunID const &theId,
                Timestamp const &theTime,
                Timestamp const &theEndTime)
      :
      processHistoryID_(),
      allEventsProcessHistories_(),
      id_(theId),
      beginTime_(theTime),
      endTime_(theEndTime)
   {}

   RunAuxiliary(RunNumber_t const &run,
                Timestamp const &theTime,
                Timestamp const &theEndTime)
      :
      processHistoryID_(),
      allEventsProcessHistories_(),
      id_(run),
      beginTime_(theTime),
      endTime_(theEndTime)
   {}

   void write(std::ostream& os) const;

   ProcessHistoryID& processHistoryID() const { return processHistoryID_; }

   void setProcessHistoryID(ProcessHistoryID const &phid) const
   { processHistoryID_ = phid; }

   RunID const &id() const { return id_; }

   Timestamp const &beginTime() const { return beginTime_; }

   Timestamp const &endTime() const { return endTime_; }

   RunID const &runID() const {return id_;}
   RunNumber_t run() const { return id_.run(); }

   void setEndTime(Timestamp const &time) {
      if (endTime_ == Timestamp::invalidTimestamp()) endTime_ = time;
   }

   bool mergeAuxiliary(RunAuxiliary const &aux);

   // most recent process that put a RunProduct into this run
   // is the last on the list, this defines what "latest" is
   mutable ProcessHistoryID processHistoryID_;

   // allEventsProcessHistories_ contains all the ProcessHistoryIDs for all
   // events in this run seen so far.
   std::set<ProcessHistoryID> allEventsProcessHistories_;

   RunID id_;
   // Times from DAQ
   Timestamp beginTime_;
   Timestamp endTime_;

private:
   void mergeNewTimestampsIntoThis_(RunAuxiliary const &newAux);
   void mergeNewProcessHistoryIntoThis_(RunAuxiliary const &newAux);
};

inline
std::ostream&
operator<<(std::ostream& os, const art::RunAuxiliary &p) {
   p.write(os);
   return os;
}

#endif /* art_Persistency_Provenance_RunAuxiliary_h */

// Local Variables:
// mode: c++
// End:
