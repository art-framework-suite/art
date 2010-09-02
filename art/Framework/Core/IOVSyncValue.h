#ifndef Framework_IOVSyncValue_h
#define Framework_IOVSyncValue_h
// -*- C++ -*-
//
// Package:     Framework
// Class  :     IOVSyncValue
//
/**\class IOVSyncValue IOVSyncValue.h FWCore/Framework/interface/IOVSyncValue.h

 Description: Provides the information needed to synchronize the EventSetup IOV with an Event

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Wed Aug  3 18:35:24 EDT 2005
//

// system include files
#include <functional>

// user include files
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/Timestamp.h"

// forward declarations

namespace edm {
class IOVSyncValue
{

   public:
      IOVSyncValue();
      //virtual ~IOVSyncValue();
      explicit IOVSyncValue(const EventID& iID, SubRunNumber_t iSubRun=0);
      explicit IOVSyncValue(const Timestamp& iTime);
      IOVSyncValue(const EventID& iID, SubRunNumber_t iSubRun, const Timestamp& iTime);

      // ---------- const member functions ---------------------
      const EventID& eventID() const { return eventID_;}
      SubRunNumber_t subRunNumber() const { return subRunID_;}
      const Timestamp& time() const {return time_; }

      bool operator==(const IOVSyncValue& iRHS) const {
         return doOp<std::equal_to>(iRHS);
      }
      bool operator!=(const IOVSyncValue& iRHS) const {
         return doOp<std::not_equal_to>(iRHS);
      }

      bool operator<(const IOVSyncValue& iRHS) const {
         return doOp<std::less>(iRHS);
      }
      bool operator<=(const IOVSyncValue& iRHS) const {
         return doOp<std::less_equal>(iRHS);
      }
      bool operator>(const IOVSyncValue& iRHS) const {
         return doOp<std::greater>(iRHS);
      }
      bool operator>=(const IOVSyncValue& iRHS) const {
         return doOp<std::greater_equal>(iRHS);
      }

      // ---------- static member functions --------------------
      static const IOVSyncValue& invalidIOVSyncValue();
      static const IOVSyncValue& endOfTime();
      static const IOVSyncValue& beginOfTime();

      // ---------- member functions ---------------------------

   private:
      //IOVSyncValue(const IOVSyncValue&); // stop default

      //const IOVSyncValue& operator=(const IOVSyncValue&); // stop default
      template< template <typename> class Op >
         bool doOp(const IOVSyncValue& iRHS) const {
            bool returnValue = false;
            if(haveID_ && iRHS.haveID_) {
               if(subRunID_==0 || iRHS.subRunID_==0 || subRunID_==iRHS.subRunID_) {
                  Op<EventID> op;
                  returnValue = op(eventID_, iRHS.eventID_);
               } else {
                  if(iRHS.eventID_.run() == eventID_.run()) {
                     Op<SubRunNumber_t> op;
                     returnValue = op(subRunID_, iRHS.subRunID_);
                  } else {
                     Op<RunNumber_t> op;
                     returnValue = op(eventID_.run(), iRHS.eventID_.run());
                  }
               }

            } else if (haveTime_ && iRHS.haveTime_) {
               Op<Timestamp> op;
               returnValue = op(time_, iRHS.time_);
            } else {
               //error
            }
            return returnValue;
         }

      // ---------- member data --------------------------------
      EventID eventID_;
      SubRunNumber_t subRunID_;
      Timestamp time_;
      bool haveID_;
      bool haveTime_;
};

}

#endif
