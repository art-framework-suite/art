#ifndef art_Persistency_Provenance_EventAuxiliary_h
#define art_Persistency_Provenance_EventAuxiliary_h

#include <iosfwd>

#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/Timestamp.h"

// Auxiliary event data that is persistent

namespace art {
   class EventAuxiliary;
}

class art::EventAuxiliary {
public:
   // These types are very tentative for now
   enum ExperimentType {
      Any = 0,
      Align = 1,
      Calib = 2,
      Cosmic = 3,
      Data = 4,
      Mc = 5,
      Raw = 6,
      Test = 7
   };

   EventAuxiliary()
      :
      id_(),
      time_(),
      isRealData_(false),
      experimentType_(Any)
   {}

   EventAuxiliary(EventID const &theId, Timestamp const &theTime,
                  bool isReal, ExperimentType eType = Any)
      :
      id_(theId),
      time_(theTime),
      isRealData_(isReal),
      experimentType_(eType)
   {}

   void write(std::ostream& os) const;

   Timestamp const &time() const { return time_; }

   EventID const &id() const { return id_; }
   RunID const &runID() const { return id_.runID(); }
   SubRunID const &subRunID() const { return id_.subRunID(); }
   RunNumber_t run() const { return id_.run(); }
   SubRunNumber_t subRun() const { return id_.subRun(); }
   EventNumber_t event() const { return id_.event(); }

   bool isRealData() const { return isRealData_; }

   ExperimentType experimentType() const { return experimentType_; }

   bool operator==(EventAuxiliary const &other) const {
      return
         id_ == other.id_ &&
         time_ == other.time_ &&
         isRealData_ == other.isRealData_ &&
         experimentType_ == other.experimentType_;
   }

private:
   // Event ID
   EventID id_;
   // Time from DAQ
   Timestamp time_;
   // Is this real data (i.e. not simulated)
   bool isRealData_;
   // Something descriptive of the source of the data
   ExperimentType experimentType_;
};


inline
std::ostream&
operator<<(std::ostream& os, const art::EventAuxiliary& p) {
   p.write(os);
   return os;
}

#endif /* art_Persistency_Provenance_EventAuxiliary_h */

// Local Variables:
// mode: c++
// End:
