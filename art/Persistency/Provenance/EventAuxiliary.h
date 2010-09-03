#ifndef DataFormats_Provenance_EventAuxiliary_h
#define DataFormats_Provenance_EventAuxiliary_h

#include <iosfwd>

#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "art/Persistency/Provenance/SubRunID.h"

// Auxiliary event data that is persistent

namespace edm
{
  struct EventAuxiliary {
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
    EventAuxiliary() :
	processHistoryID_(),
	id_(),
        processGUID_(),
	time_(),
	subRun_(),
	isRealData_(false),
	experimentType_(Any),
	bunchCrossing_(invalidBunchXing),
	orbitNumber_(invalidBunchXing),
        storeNumber_(invalidStoreNumber) {}
    EventAuxiliary(EventID const& theId, std::string const& theProcessGUID, Timestamp const& theTime,
		   SubRunNumber_t lb, bool isReal, ExperimentType eType = Any,
		   int bunchXing = invalidBunchXing, int storeNumber = invalidStoreNumber,
                   int orbitNum = invalidBunchXing) :
	processHistoryID_(),
	id_(theId),
        processGUID_(theProcessGUID),
	time_(theTime),
	subRun_(lb),
	isRealData_(isReal),
        experimentType_(eType),
	bunchCrossing_(bunchXing),
	orbitNumber_(orbitNum),
	storeNumber_(storeNumber) {}
    ~EventAuxiliary() {}
    void write(std::ostream& os) const;
    ProcessHistoryID& processHistoryID() const {return processHistoryID_;}
    EventID const& id() const {return id_;}
    std::string const& processGUID() const {return processGUID_;}
    Timestamp const& time() const {return time_;}
    SubRunNumber_t const& subRun() const {return subRun_;}
    EventNumber_t event() const {return id_.event();}
    RunNumber_t run() const {return id_.run();}
    bool isRealData() const {return isRealData_;}
    ExperimentType experimentType() const {return experimentType_;}
    int bunchCrossing() const {return bunchCrossing_;}
    int orbitNumber() const {return orbitNumber_;}
    int storeNumber() const {return storeNumber_;}

    // most recently process that processed this event
    // is the last on the list, this defines what "latest" is
    mutable ProcessHistoryID processHistoryID_;
    // Event ID
    EventID id_;
    // Globally unique process ID of process that created event.
    std::string processGUID_;
    // Time from DAQ
    Timestamp time_;
    // Associated SubRun identifier
    SubRunNumber_t subRun_;
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

  bool
  isSameEvent(EventAuxiliary const& a, EventAuxiliary const& b);

  inline
  std::ostream&
  operator<<(std::ostream& os, const EventAuxiliary& p) {
    p.write(os);
    return os;
  }

}

#endif
