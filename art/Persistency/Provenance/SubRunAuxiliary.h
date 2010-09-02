#ifndef DataFormats_Provenance_SubRunAuxiliary_h
#define DataFormats_Provenance_SubRunAuxiliary_h

#include <iosfwd>

#include "art/Persistency/Provenance/ProcessHistoryID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/Timestamp.h"

// Auxiliary luminosity block data that is persistent

namespace edm
{
  struct SubRunAuxiliary {
    SubRunAuxiliary() :
	processHistoryID_(),
	id_(),
	beginTime_(),
	endTime_() {}
    SubRunAuxiliary(SubRunID const& theId,
			     Timestamp const& theTime,
			     Timestamp const& theEndTime) :
	processHistoryID_(),
	id_(theId),
	beginTime_(theTime),
	endTime_(theEndTime) {}
    SubRunAuxiliary(RunNumber_t const& theRun,
			     SubRunNumber_t const& theLumi,
			     Timestamp const& theTime,
			     Timestamp const& theEndTime) :
	processHistoryID_(),
	id_(theRun, theLumi),
	beginTime_(theTime),
	endTime_(theEndTime) {}
    ~SubRunAuxiliary() {}
    void write(std::ostream& os) const;
    ProcessHistoryID& processHistoryID() const {return processHistoryID_;}
    void setProcessHistoryID(ProcessHistoryID const& phid) const {processHistoryID_ = phid;}
    SubRunNumber_t luminosityBlock() const {return id().luminosityBlock();}
    RunNumber_t run() const {return id().run();}
    SubRunID const& id() const {return id_;}
    Timestamp const& beginTime() const {return beginTime_;}
    Timestamp const& endTime() const {return endTime_;}
    void setEndTime(Timestamp const& time) {
      if (endTime_ == Timestamp::invalidTimestamp()) endTime_ = time;
    }
    bool mergeAuxiliary(SubRunAuxiliary const& newAux);

    // most recent process that processed this lumi block
    // is the last on the list, this defines what "latest" is
    mutable ProcessHistoryID processHistoryID_;
    // SubRun ID
    SubRunID id_;
    // Times from DAQ
    Timestamp beginTime_;
    Timestamp endTime_;
  };

  inline
  std::ostream&
  operator<<(std::ostream& os, const SubRunAuxiliary& p) {
    p.write(os);
    return os;
  }

}
#endif
