// -*- C++ -*-
//
// Package:     Framework
// Class  :     intersectingiovrecordintervalfinder_t_cppunit
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Tue Aug 19 14:14:42 EDT 2008
//
//

// system include files

// user include files
#include "art/Framework/Core/IntersectingIOVRecordIntervalFinder.h"
#include "test/Framework/Core/DummyRecord.h"
#include "test/Framework/Core/DummyFinder.h"

#include <cppunit/extensions/HelperMacros.h>
using namespace art::eventsetup;

class testintersectingiovrecordintervalfinder: public CppUnit::TestFixture
{
   CPPUNIT_TEST_SUITE(testintersectingiovrecordintervalfinder);

   CPPUNIT_TEST(constructorTest);
   CPPUNIT_TEST(intersectionTest);

   CPPUNIT_TEST_SUITE_END();
public:

   void setUp(){}
   void tearDown(){}

   void constructorTest();
   void intersectionTest();

}; //Cppunit class declaration over

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testintersectingiovrecordintervalfinder);
namespace  {
   class DepRecordFinder : public art::EventSetupRecordIntervalFinder {
   public:
      DepRecordFinder() :art::EventSetupRecordIntervalFinder(), interval_() {
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
}

void
testintersectingiovrecordintervalfinder::constructorTest()
{
   IntersectingIOVRecordIntervalFinder finder(DummyRecord::keyForClass());
   CPPUNIT_ASSERT(finder.findingForRecords().size() == 1);
   std::set<EventSetupRecordKey> s = finder.findingForRecords();
   CPPUNIT_ASSERT(s.find(DummyRecord::keyForClass()) != s.end());
}

void
testintersectingiovrecordintervalfinder::intersectionTest()
{
   const EventSetupRecordKey dummyRecordKey = DummyRecord::keyForClass();

   IntersectingIOVRecordIntervalFinder intFinder(dummyRecordKey);
   std::vector<boost::shared_ptr<art::EventSetupRecordIntervalFinder> > finders;
   boost::shared_ptr<DummyFinder> dummyFinder(new DummyFinder);
   {
      const art::EventID eID_1(1, 1);
      const art::IOVSyncValue sync_1(eID_1);
      const art::EventID eID_3(1, 3);
      const art::ValidityInterval definedInterval(sync_1,
                                                  art::IOVSyncValue(eID_3));
      finders.push_back(dummyFinder);
      dummyFinder->setInterval(definedInterval);
      intFinder.swapFinders(finders);

      CPPUNIT_ASSERT(definedInterval == intFinder.findIntervalFor(dummyRecordKey, art::IOVSyncValue(art::EventID(1, 2))));
   }

   {
      const art::EventID eID_5(1, 5);
      const art::IOVSyncValue sync_5(eID_5);
      const art::ValidityInterval unknownedEndInterval(sync_5 ,
                                                       art::IOVSyncValue::invalidIOVSyncValue());
      dummyFinder->setInterval(unknownedEndInterval);

      CPPUNIT_ASSERT(unknownedEndInterval == intFinder.findIntervalFor(dummyRecordKey, art::IOVSyncValue(art::EventID(1, 5))));
   }

   {
      finders.clear();

      const art::EventID eID_1(1, 1);
      const art::IOVSyncValue sync_1(eID_1);
      const art::EventID eID_3(1, 3);
      const art::IOVSyncValue sync_3(eID_3);
      const art::EventID eID_4(1, 4);
      const art::IOVSyncValue sync_4(eID_4);
      const art::EventID eID_5(1, 5);
      const art::IOVSyncValue sync_5(eID_5);
      const art::ValidityInterval definedInterval(sync_1,
                                                  sync_4);
      dummyFinder->setInterval(definedInterval);
      finders.push_back(dummyFinder);

      boost::shared_ptr<DummyFinder> dummyFinder2(new DummyFinder);
      dummyFinder2->setInterval(art::ValidityInterval(sync_3, sync_5));
      finders.push_back(dummyFinder2);
      IntersectingIOVRecordIntervalFinder intFinder(dummyRecordKey);
      intFinder.swapFinders(finders);

      CPPUNIT_ASSERT(art::ValidityInterval(sync_3,sync_4) ==
                     intFinder.findIntervalFor(dummyRecordKey, sync_3));
   }

   {
      finders.clear();
      const art::EventID eID_1(1, 1);
      const art::IOVSyncValue sync_1(eID_1);
      const art::EventID eID_3(1, 3);
      const art::IOVSyncValue sync_3(eID_3);
      const art::EventID eID_4(1, 4);
      const art::IOVSyncValue sync_4(eID_4);
      const art::EventID eID_5(1, 5);
      const art::IOVSyncValue sync_5(eID_5);
      const art::ValidityInterval definedInterval(sync_1,
                                                  sync_4);
      dummyFinder->setInterval(definedInterval);
      finders.push_back(dummyFinder);

      boost::shared_ptr<DummyFinder> dummyFinder2(new DummyFinder);
      dummyFinder2->setInterval(art::ValidityInterval::invalidInterval());
      finders.push_back(dummyFinder2);
      IntersectingIOVRecordIntervalFinder intFinder(dummyRecordKey);
      intFinder.swapFinders(finders);

      CPPUNIT_ASSERT(art::ValidityInterval::invalidInterval() ==
                     dummyFinder2->findIntervalFor(dummyRecordKey,sync_3));

      CPPUNIT_ASSERT(art::ValidityInterval(sync_1,art::IOVSyncValue::invalidIOVSyncValue()) ==
                     intFinder.findIntervalFor(dummyRecordKey, sync_3));
   }

   {
      finders.clear();
      const art::EventID eID_1(1, 1);
      const art::IOVSyncValue sync_1(eID_1);
      const art::EventID eID_3(1, 3);
      const art::IOVSyncValue sync_3(eID_3);
      const art::EventID eID_4(1, 4);
      const art::IOVSyncValue sync_4(eID_4);
      const art::ValidityInterval definedInterval(sync_1,
                                                  sync_4);
      dummyFinder->setInterval(definedInterval);
      finders.push_back(dummyFinder);

      boost::shared_ptr<DummyFinder> dummyFinder2(new DummyFinder);
      dummyFinder2->setInterval(art::ValidityInterval(sync_3, art::IOVSyncValue::invalidIOVSyncValue()));
      finders.push_back(dummyFinder2);
      IntersectingIOVRecordIntervalFinder intFinder(dummyRecordKey);
      intFinder.swapFinders(finders);

      CPPUNIT_ASSERT(art::ValidityInterval(sync_3,art::IOVSyncValue::invalidIOVSyncValue()) ==
                     dummyFinder2->findIntervalFor(dummyRecordKey,sync_3));

      CPPUNIT_ASSERT(art::ValidityInterval(sync_3,art::IOVSyncValue::invalidIOVSyncValue()) ==
                     intFinder.findIntervalFor(dummyRecordKey, sync_3));
   }

   {
      //reverse order so invalid ending is first in list
      finders.clear();
      const art::EventID eID_1(1, 1);
      const art::IOVSyncValue sync_1(eID_1);
      const art::EventID eID_3(1, 3);
      const art::IOVSyncValue sync_3(eID_3);
      const art::EventID eID_4(1, 4);
      const art::IOVSyncValue sync_4(eID_4);
      const art::ValidityInterval definedInterval(sync_1,
                                                  sync_4);

      boost::shared_ptr<DummyFinder> dummyFinder2(new DummyFinder);
      dummyFinder2->setInterval(art::ValidityInterval(sync_3, art::IOVSyncValue::invalidIOVSyncValue()));
      finders.push_back(dummyFinder2);

      dummyFinder->setInterval(definedInterval);
      finders.push_back(dummyFinder);

      IntersectingIOVRecordIntervalFinder intFinder(dummyRecordKey);
      intFinder.swapFinders(finders);

      CPPUNIT_ASSERT(art::ValidityInterval(sync_3,art::IOVSyncValue::invalidIOVSyncValue()) ==
                     dummyFinder2->findIntervalFor(dummyRecordKey,sync_3));

      CPPUNIT_ASSERT(art::ValidityInterval(sync_3,art::IOVSyncValue::invalidIOVSyncValue()) ==
                     intFinder.findIntervalFor(dummyRecordKey, sync_3));
   }

}
