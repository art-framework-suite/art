#ifndef art_Persistency_Provenance_SubRunAuxiliary_h
#define art_Persistency_Provenance_SubRunAuxiliary_h

#include <iosfwd>

#include "canvas/Persistency/Provenance/ProcessHistoryID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/Timestamp.h"

// Auxiliary subRun data that is persistent

namespace art {
   class SubRunAuxiliary;
}

class art::SubRunAuxiliary {
public:
   SubRunAuxiliary()
      :
      processHistoryID_(),
      id_(),
      beginTime_(),
      endTime_()
   {}

   SubRunAuxiliary(SubRunID const &theId,
                   Timestamp const &theTime,
                   Timestamp const &theEndTime)
      :
      processHistoryID_(),
      id_(theId),
      beginTime_(theTime),
      endTime_(theEndTime)
   {}

   SubRunAuxiliary(RunNumber_t const &theRun,
                   SubRunNumber_t const &theSubRun,
                   Timestamp const &theTime,
                   Timestamp const &theEndTime)
      :
      processHistoryID_(),
      id_(theRun, theSubRun),
      beginTime_(theTime),
      endTime_(theEndTime)
   {}

   void write(std::ostream& os) const;

   ProcessHistoryID& processHistoryID() const { return processHistoryID_; }

   void setProcessHistoryID(ProcessHistoryID const &phid) const { processHistoryID_ = phid; }

   SubRunID const &id() const { return id_; }
   RunID const &runID() const { return id_.runID(); }
   RunNumber_t run() const { return id_.run(); }
   SubRunNumber_t subRun() const { return id_.subRun(); }

   Timestamp const &beginTime() const { return beginTime_; }

   Timestamp const &endTime() const { return endTime_; }

   void setEndTime(Timestamp const &time) {
      if (endTime_ == Timestamp::invalidTimestamp()) endTime_ = time;
   }

   bool mergeAuxiliary(SubRunAuxiliary const &newAux);

   // most recent process that processed this subRun
   // is the last on the list, this defines what "latest" is
   mutable ProcessHistoryID processHistoryID_;

   SubRunID id_;
   // Times from DAQ
   Timestamp beginTime_;
   Timestamp endTime_;
};

inline
std::ostream&
operator<<(std::ostream& os, const art::SubRunAuxiliary& p) {
   p.write(os);
   return os;
}

#endif /* art_Persistency_Provenance_SubRunAuxiliary_h */

// Local Variables:
// mode: c++
// End:
