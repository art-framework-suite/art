#ifndef Framework_DummyFinder_h
#define Framework_DummyFinder_h
// -*- C++ -*-
//
// Package:     Framework
// Class  :     DummyFinder
//
/**\class DummyFinder DummyFinder.h FWCore/Framework/interface/DummyFinder.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Author:      Chris Jones
// Created:     Sat Apr 16 18:47:04 EDT 2005
//
//

// system include files

// user include files
#include "art/Framework/Core/EventSetupRecordIntervalFinder.h"
#include "test/Framework/Core/DummyRecord.h"

// forward declarations

class DummyFinder : public art::EventSetupRecordIntervalFinder {
public:
   DummyFinder() :art::EventSetupRecordIntervalFinder(), interval_() {
      this->findingRecord<DummyRecord>();
   }

   void setInterval(const art::ValidityInterval& iInterval) {
      interval_ = iInterval;
   }
protected:
   virtual void setIntervalFor(const art::eventsetup::EventSetupRecordKey&,
                                const art::IOVSyncValue& iTime,
                                art::ValidityInterval& iInterval) {
      if(interval_.validFor(iTime)) {
         iInterval = interval_;
      } else {
         if(interval_.last() == art::IOVSyncValue::invalidIOVSyncValue() &&
             interval_.first() != art::IOVSyncValue::invalidIOVSyncValue() &&
             interval_.first() <= iTime) {
            iInterval = interval_;
         }else {
            iInterval = art::ValidityInterval();
         }
      }
   }
private:
   art::ValidityInterval interval_;
};


#endif
