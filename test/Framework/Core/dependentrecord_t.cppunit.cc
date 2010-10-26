/*
 *  dependentrecord_t.cpp
 *  EDMProto
 *
 *  Created by Chris Jones on 4/29/05.
 *  Changed by Viji Sundararajan on 29-Jun-2005
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "boost/mpl/vector.hpp"
#include "art/Framework/Core/DependentRecordImplementation.h"
#include "test/Framework/Core/DummyRecord.h"
#include "test/Framework/Core/Dummy2Record.h"
#include "test/Framework/Core/DepRecord.h"
#include "test/Framework/Core/DepOn2Record.h"
#include "test/Framework/Core/DummyFinder.h"
#include "art/Framework/Core/EventSetupRecordProviderFactoryManager.h"
#include "art/Framework/Core/DependentRecordIntervalFinder.h"
#include "art/Framework/Core/EventSetupProvider.h"
#include "art/Framework/Core/DataProxyProvider.h"
#include "art/Framework/Core/EventSetupRecordProvider.h"
#include "art/Framework/Core/NoRecordException.h"

#include <cppunit/extensions/HelperMacros.h>
#include <string.h>


using namespace art::eventsetup;

class testdependentrecord: public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(testdependentrecord);

CPPUNIT_TEST(dependentConstructorTest);
CPPUNIT_TEST(dependentFinder1Test);
CPPUNIT_TEST(dependentFinder2Test);
CPPUNIT_TEST(dependentSetproviderTest);
CPPUNIT_TEST(getTest);
CPPUNIT_TEST(oneOfTwoRecordTest);
CPPUNIT_TEST(resetTest);
CPPUNIT_TEST(alternateFinderTest);

CPPUNIT_TEST_SUITE_END();
public:

  void setUp(){}
  void tearDown(){}

  void dependentConstructorTest();
  void dependentFinder1Test();
  void dependentFinder2Test();
  void dependentSetproviderTest();
  void getTest();
  void oneOfTwoRecordTest();
  void resetTest();
  void alternateFinderTest();

}; //Cppunit class declaration over

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testdependentrecord);

namespace {
class DummyProxyProvider : public art::eventsetup::DataProxyProvider {
public:
   DummyProxyProvider() {
      usingRecord<DummyRecord>();
   }
   void newInterval(const art::eventsetup::EventSetupRecordKey& /*iRecordType*/,
                     const art::ValidityInterval& /*iInterval*/) {
      //do nothing
   }
protected:
   void registerProxies(const art::eventsetup::EventSetupRecordKey&, KeyedProxies& /*iHolder*/) {
   }

};

class DepRecordProxyProvider : public art::eventsetup::DataProxyProvider {
public:
   DepRecordProxyProvider() {
      usingRecord<DepRecord>();
   }
   void newInterval(const art::eventsetup::EventSetupRecordKey& /*iRecordType*/,
                     const art::ValidityInterval& /*iInterval*/) {
      //do nothing
   }
protected:
   void registerProxies(const art::eventsetup::EventSetupRecordKey&, KeyedProxies& /*iHolder*/) {
   }

};


class DepOn2RecordProxyProvider : public art::eventsetup::DataProxyProvider {
public:
  DepOn2RecordProxyProvider() {
    usingRecord<DepOn2Record>();
  }
  void newInterval(const art::eventsetup::EventSetupRecordKey& /*iRecordType*/,
  const art::ValidityInterval& /*iInterval*/) {
    //do nothing
  }
protected:
void registerProxies(const art::eventsetup::EventSetupRecordKey&, KeyedProxies& /*iHolder*/) {
}

};

class DepRecordFinder : public art::EventSetupRecordIntervalFinder {
public:
  DepRecordFinder() :art::EventSetupRecordIntervalFinder(), interval_() {
    this->findingRecord<DepRecord>();
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
}

using namespace art::eventsetup;
void testdependentrecord::dependentConstructorTest()
{
   std::auto_ptr<EventSetupRecordProvider> depProvider =
   EventSetupRecordProviderFactoryManager::instance().makeRecordProvider(DepRecord::keyForClass());

   CPPUNIT_ASSERT(1 == depProvider->dependentRecords().size());
   CPPUNIT_ASSERT(*(depProvider->dependentRecords().begin()) == DummyRecord::keyForClass());
}


void testdependentrecord::dependentFinder1Test()
{
   boost::shared_ptr<EventSetupRecordProvider> dummyProvider(
                                                          EventSetupRecordProviderFactoryManager::instance()
                                                              .makeRecordProvider(DummyRecord::keyForClass()).release());
   const art::EventID eID_1(1, 1);
   const art::IOVSyncValue sync_1(eID_1);
   const art::EventID eID_3(1, 3);
   const art::ValidityInterval definedInterval(sync_1,
                                                art::IOVSyncValue(eID_3));
   boost::shared_ptr<DummyFinder> dummyFinder(new DummyFinder);
   dummyFinder->setInterval(definedInterval);
   dummyProvider->addFinder(dummyFinder);

   const EventSetupRecordKey depRecordKey = DepRecord::keyForClass();
   DependentRecordIntervalFinder finder(depRecordKey);
   finder.addProviderWeAreDependentOn(dummyProvider);

   CPPUNIT_ASSERT(definedInterval == finder.findIntervalFor(depRecordKey, art::IOVSyncValue(art::EventID(1, 2))));

   dummyFinder->setInterval(art::ValidityInterval::invalidInterval());
   CPPUNIT_ASSERT(art::ValidityInterval::invalidInterval() == finder.findIntervalFor(depRecordKey,
                                                                                     art::IOVSyncValue(art::EventID(1, 4))));

   const art::EventID eID_5(1, 5);
   const art::IOVSyncValue sync_5(eID_5);
   const art::ValidityInterval unknownedEndInterval(sync_5 ,
                                                     art::IOVSyncValue::invalidIOVSyncValue());
   dummyFinder->setInterval(unknownedEndInterval);

   CPPUNIT_ASSERT(unknownedEndInterval == finder.findIntervalFor(depRecordKey, art::IOVSyncValue(art::EventID(1, 5))));

}

void testdependentrecord::dependentFinder2Test()
{
   boost::shared_ptr<EventSetupRecordProvider> dummyProvider1(EventSetupRecordProviderFactoryManager::instance()
                                                              .makeRecordProvider(DummyRecord::keyForClass()).release());

   const art::EventID eID_1(1, 1);
   const art::IOVSyncValue sync_1(eID_1);
   const art::ValidityInterval definedInterval1(sync_1,
                                                 art::IOVSyncValue(art::EventID(1, 5)));
   dummyProvider1->setValidityInterval(definedInterval1);

   boost::shared_ptr<EventSetupRecordProvider> dummyProvider2(EventSetupRecordProviderFactoryManager::instance()
                                                              .makeRecordProvider(DummyRecord::keyForClass()).release());

   const art::EventID eID_2(1, 2);
   const art::IOVSyncValue sync_2(eID_2);
   const art::ValidityInterval definedInterval2(sync_2,
                                                 art::IOVSyncValue(art::EventID(1, 6)));
   dummyProvider2->setValidityInterval(definedInterval2);

   const art::ValidityInterval overlapInterval(std::max(definedInterval1.first(), definedInterval2.first()),
                                                std::min(definedInterval1.last(), definedInterval2.last()));

   const EventSetupRecordKey depRecordKey = DepRecord::keyForClass();

   DependentRecordIntervalFinder finder(depRecordKey);
   finder.addProviderWeAreDependentOn(dummyProvider1);
   finder.addProviderWeAreDependentOn(dummyProvider2);

   CPPUNIT_ASSERT(overlapInterval == finder.findIntervalFor(depRecordKey,
                                                            art::IOVSyncValue(art::EventID(1, 4))));
}


void testdependentrecord::dependentSetproviderTest()
{
   std::auto_ptr<EventSetupRecordProvider> depProvider =
   EventSetupRecordProviderFactoryManager::instance().makeRecordProvider(DepRecord::keyForClass());

   boost::shared_ptr<EventSetupRecordProvider> dummyProvider(
       EventSetupRecordProviderFactoryManager::instance().makeRecordProvider(DummyRecord::keyForClass()).release());

   boost::shared_ptr<DummyFinder> dummyFinder(new DummyFinder);
   dummyFinder->setInterval(art::ValidityInterval(art::IOVSyncValue(art::EventID(1, 1)),
                                                   art::IOVSyncValue(art::EventID(1, 3))));
   dummyProvider->addFinder(dummyFinder);

   CPPUNIT_ASSERT(*(depProvider->dependentRecords().begin()) == dummyProvider->key());

   std::vector< boost::shared_ptr<EventSetupRecordProvider> > providers;
   providers.push_back(dummyProvider);
   depProvider->setDependentProviders(providers);
}

void testdependentrecord::getTest()
{
   art::eventsetup::EventSetupProvider provider;
   boost::shared_ptr<art::eventsetup::DataProxyProvider> dummyProv(new DummyProxyProvider());
   provider.add(dummyProv);

   boost::shared_ptr<DummyFinder> dummyFinder(new DummyFinder);
   dummyFinder->setInterval(art::ValidityInterval(art::IOVSyncValue(art::EventID(1, 1)),
                                                   art::IOVSyncValue(art::EventID(1, 3))));
   provider.add(boost::shared_ptr<art::EventSetupRecordIntervalFinder>(dummyFinder));

   boost::shared_ptr<art::eventsetup::DataProxyProvider> depProv(new DepRecordProxyProvider());
   provider.add(depProv);
   {
      const art::EventSetup& eventSetup = provider.eventSetupForInstance(art::IOVSyncValue(art::EventID(1, 1)));
      const DepRecord& depRecord = eventSetup.get<DepRecord>();

      depRecord.getRecord<DummyRecord>();
   }
   {
      const art::EventSetup& eventSetup = provider.eventSetupForInstance(art::IOVSyncValue(art::EventID(1, 4)));
      CPPUNIT_ASSERT_THROW(eventSetup.get<DepRecord>(),art::eventsetup::NoRecordException<DepRecord>);
   }

}

void testdependentrecord::oneOfTwoRecordTest()
{
  art::eventsetup::EventSetupProvider provider;
  boost::shared_ptr<art::eventsetup::DataProxyProvider> dummyProv(new DummyProxyProvider());
  provider.add(dummyProv);

  boost::shared_ptr<DummyFinder> dummyFinder(new DummyFinder);
  dummyFinder->setInterval(art::ValidityInterval(art::IOVSyncValue(art::EventID(1, 1)),
                                                 art::IOVSyncValue(art::EventID(1, 3))));
  provider.add(boost::shared_ptr<art::EventSetupRecordIntervalFinder>(dummyFinder));

  boost::shared_ptr<art::eventsetup::DataProxyProvider> depProv(new DepOn2RecordProxyProvider());
  provider.add(depProv);
  {
    const art::EventSetup& eventSetup = provider.eventSetupForInstance(art::IOVSyncValue(art::EventID(1, 1)));
    const DepOn2Record& depRecord = eventSetup.get<DepOn2Record>();

    depRecord.getRecord<DummyRecord>();
    CPPUNIT_ASSERT_THROW(depRecord.getRecord<Dummy2Record>(),art::eventsetup::NoRecordException<Dummy2Record>);

    try {
      depRecord.getRecord<Dummy2Record>();
    } catch(art::eventsetup::NoRecordException<Dummy2Record>& e) {
       //make sure that the record name appears in the error message.
       CPPUNIT_ASSERT(0!=strstr(e.what(), "DepOn2Record"));
       CPPUNIT_ASSERT(0!=strstr(e.what(), "Dummy2Record"));
       //	std::cout<<e.what()<<std::endl;
    }
  }
}
void testdependentrecord::resetTest()
{
  art::eventsetup::EventSetupProvider provider;
  boost::shared_ptr<art::eventsetup::DataProxyProvider> dummyProv(new DummyProxyProvider());
  provider.add(dummyProv);

  boost::shared_ptr<DummyFinder> dummyFinder(new DummyFinder);
  dummyFinder->setInterval(art::ValidityInterval(art::IOVSyncValue(art::EventID(1, 1)),
                                                 art::IOVSyncValue(art::EventID(1, 3))));
  provider.add(boost::shared_ptr<art::EventSetupRecordIntervalFinder>(dummyFinder));

  boost::shared_ptr<art::eventsetup::DataProxyProvider> depProv(new DepRecordProxyProvider());
  provider.add(depProv);
  {
    const art::EventSetup& eventSetup = provider.eventSetupForInstance(art::IOVSyncValue(art::EventID(1, 1)));
    const DepRecord& depRecord = eventSetup.get<DepRecord>();
    unsigned long long depCacheID = depRecord.cacheIdentifier();
    const DummyRecord& dummyRecord = depRecord.getRecord<DummyRecord>();
    unsigned long long dummyCacheID = dummyRecord.cacheIdentifier();

    provider.resetRecordPlusDependentRecords(dummyRecord.key());
    CPPUNIT_ASSERT(dummyCacheID != dummyRecord.cacheIdentifier());
    CPPUNIT_ASSERT(depCacheID != depRecord.cacheIdentifier());
  }
}
void testdependentrecord::alternateFinderTest()
{
  boost::shared_ptr<EventSetupRecordProvider> dummyProvider(
                                                            EventSetupRecordProviderFactoryManager::instance()
                                                            .makeRecordProvider(DummyRecord::keyForClass()).release());
  const art::EventID eID_1(1, 1);
  const art::IOVSyncValue sync_1(eID_1);
  const art::EventID eID_3(1, 3);
  const art::IOVSyncValue sync_3(eID_3);
  const art::EventID eID_4(1, 4);
  const art::ValidityInterval definedInterval(sync_1,
                                              art::IOVSyncValue(eID_4));
  boost::shared_ptr<DummyFinder> dummyFinder(new DummyFinder);
  dummyFinder->setInterval(definedInterval);
  dummyProvider->addFinder(dummyFinder);

  boost::shared_ptr<DepRecordFinder> depFinder(new DepRecordFinder);
  const art::EventID eID_2(1, 2);
  const art::IOVSyncValue sync_2(eID_2);
  const art::ValidityInterval depInterval(sync_1,
                                          sync_2);
  depFinder->setInterval(depInterval);

  const EventSetupRecordKey depRecordKey = DepRecord::keyForClass();
  DependentRecordIntervalFinder finder(depRecordKey);
  finder.setAlternateFinder(depFinder);
  finder.addProviderWeAreDependentOn(dummyProvider);

  CPPUNIT_ASSERT(depInterval == finder.findIntervalFor(depRecordKey, art::IOVSyncValue(art::EventID(1, 1))));

  const art::ValidityInterval dep2Interval(sync_3,
                                           art::IOVSyncValue(eID_4));
  depFinder->setInterval(dep2Interval);
  const art::ValidityInterval tempIOV =finder.findIntervalFor(depRecordKey, sync_3);
  //std::cout <<  tempIOV.first().eventID()<<" to "<<tempIOV.last().eventID() <<std::endl;
  CPPUNIT_ASSERT(dep2Interval == finder.findIntervalFor(depRecordKey, sync_3));

  dummyFinder->setInterval(art::ValidityInterval::invalidInterval());
  depFinder->setInterval(art::ValidityInterval::invalidInterval());
  CPPUNIT_ASSERT(art::ValidityInterval::invalidInterval() == finder.findIntervalFor(depRecordKey,
                                                                                    art::IOVSyncValue(art::EventID(1, 5))));

  const art::EventID eID_6(1, 6);
  const art::IOVSyncValue sync_6(eID_6);
  const art::ValidityInterval unknownedEndInterval(sync_6 ,
                                                   art::IOVSyncValue::invalidIOVSyncValue());
  dummyFinder->setInterval(unknownedEndInterval);

  const art::EventID eID_7(1, 7);
  const art::IOVSyncValue sync_7(eID_7);
  const art::ValidityInterval iov6_7(sync_6,sync_7);
  depFinder->setInterval(iov6_7);

  CPPUNIT_ASSERT(unknownedEndInterval == finder.findIntervalFor(depRecordKey, sync_6));

  //see if dependent record can override the finder
  dummyFinder->setInterval(depInterval);
  depFinder->setInterval(definedInterval);
  CPPUNIT_ASSERT(depInterval == finder.findIntervalFor(depRecordKey, art::IOVSyncValue(art::EventID(1, 1))));

  dummyFinder->setInterval(dep2Interval);
  CPPUNIT_ASSERT(dep2Interval == finder.findIntervalFor(depRecordKey, sync_3));
}
