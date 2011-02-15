#ifndef art_Persistency_Provenance_EventAuxiliary_h
#define art_Persistency_Provenance_EventAuxiliary_h

#include <iosfwd>

#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/Timestamp.h"

// Auxiliary event data that is persistent

namespace art {
   class EventAuxiliary;

   bool
   isSameEvent(EventAuxiliary const &left, EventAuxiliary const &right);
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

   static int const invalidBunchXing = -1;

   static int const invalidStoreNumber = 0;

   EventAuxiliary()
      :
      processHistoryID_(),
      id_(),
      processGUID_(),
      time_(),
      isRealData_(false),
      experimentType_(Any),
      bunchCrossing_(invalidBunchXing),
      orbitNumber_(invalidBunchXing),
      storeNumber_(invalidStoreNumber)
   {}

   EventAuxiliary(EventID const &theId, std::string const &theProcessGUID, Timestamp const &theTime,
                  bool isReal, ExperimentType eType = Any,
                  int bunchXing = invalidBunchXing, int storeNumber = invalidStoreNumber,
                  int orbitNum = invalidBunchXing)
      :
      processHistoryID_(),
      id_(theId),
      processGUID_(theProcessGUID),
      time_(theTime),
      isRealData_(isReal),
      experimentType_(eType),
      bunchCrossing_(bunchXing),
      orbitNumber_(orbitNum),
      storeNumber_(storeNumber)
   {}

   void write(std::ostream& os) const;

   ProcessHistoryID& processHistoryID() const { return processHistoryID_; }

   std::string const &processGUID() const { return processGUID_; }

   Timestamp const &time() const { return time_; }

   EventID const &id() const { return id_; }
   RunID const &runID() const { return id_.runID(); }
   SubRunID const &subRunID() const { return id_.subRunID(); }
   RunNumber_t run() const { return id_.run(); }
   SubRunNumber_t subRun() const { return id_.subRun(); }
   EventNumber_t event() const { return id_.event(); }

   bool isRealData() const { return isRealData_; }

   ExperimentType experimentType() const { return experimentType_; }

   int bunchCrossing() const { return bunchCrossing_; }

   int orbitNumber() const { return orbitNumber_; }

   int storeNumber() const { return storeNumber_; }

   // most recently process that processed this event
   // is the last on the list, this defines what "latest" is
   mutable ProcessHistoryID processHistoryID_;

   // Event ID
   EventID id_;
   // Globally unique process ID of process that created event.
   std::string processGUID_;
   // Time from DAQ
   Timestamp time_;
   // Is this real data (i.e. not simulated)
   bool isRealData_;
   // Something descriptive of the source of the data
   ExperimentType experimentType_;
   //  The bunch crossing number
   int bunchCrossing_;
   // The orbit number
   int orbitNumber_;
   //  The LHC store number
   int storeNumber_;
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
