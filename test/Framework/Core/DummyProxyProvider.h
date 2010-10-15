#ifndef Framework_DummyProvider_h
#define Framework_DummyProvider_h
// -*- C++ -*-
//
// Package:     Framework
// Class  :     DummyProvider
//
/**\class DummyProvider DummyProvider.h FWCore/Framework/interface/DummyProvider.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Author:      Chris Jones
// Created:     Thu May 26 13:37:48 EDT 2005
//
//

// system include files

// user include files
#include "test/Framework/Core/DummyRecord.h"
#include "test/Framework/Core/DummyData.h"

#include "art/Framework/Core/DataProxyTemplate.h"
#include "art/Framework/Core/DataProxyProvider.h"

// forward declarations
namespace art {
   namespace eventsetup {
      namespace test {
class WorkingDummyProxy : public art::eventsetup::DataProxyTemplate<DummyRecord, DummyData> {
public:
   WorkingDummyProxy(const DummyData* iDummy) : data_(iDummy) {}

protected:

   const value_type* make(const record_type&, const DataKey&) {
      return data_ ;
   }
   void invalidateCache() {
   }
private:
   const DummyData* data_;
};


class DummyProxyProvider : public art::eventsetup::DataProxyProvider {
public:
   DummyProxyProvider(const DummyData& iData=DummyData()): dummy_(iData) {
      //std::cout <<"constructed provider"<<std::endl;
      usingRecord<DummyRecord>();
   }
   void newInterval(const eventsetup::EventSetupRecordKey& /*iRecordType*/,
                     const ValidityInterval& /*iInterval*/) {
      //do nothing
   }
protected:
   void registerProxies(const eventsetup::EventSetupRecordKey&, KeyedProxies& iProxies) {
      //std::cout <<"registered proxy"<<std::endl;

      boost::shared_ptr<WorkingDummyProxy> pProxy(new WorkingDummyProxy(&dummy_));
      insertProxy(iProxies, pProxy);
   }

private:
   DummyData dummy_;
};

      }
   }
}
#endif
