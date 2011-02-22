/*
 *  eventid_t.cppunit.cc
 *  CMSSW
 *
 *  Created by Chris Jones on 8/8/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include <cppunit/extensions/HelperMacros.h>

#include "art/Persistency/Provenance/Timestamp.h"

using namespace art;

class testTimestamp: public CppUnit::TestFixture
{
   CPPUNIT_TEST_SUITE(testTimestamp);

   CPPUNIT_TEST(constructTest);
   CPPUNIT_TEST(comparisonTest);

   CPPUNIT_TEST_SUITE_END();
public:
      void setUp(){}
   void tearDown(){}

   void constructTest();
   void comparisonTest();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testTimestamp);


void testTimestamp::constructTest()
{
   const TimeValue_t t = 2;

   Timestamp temp(t);

   CPPUNIT_ASSERT(temp.value() == t);

   CPPUNIT_ASSERT(Timestamp::invalidTimestamp() < Timestamp::beginOfTime());
   CPPUNIT_ASSERT(Timestamp::beginOfTime() < Timestamp::endOfTime());
   CPPUNIT_ASSERT(Timestamp::endOfTime().value() + 1 == 0);

   Timestamp db(0xdeadbeefbeefdead);

   CPPUNIT_ASSERT(db.timeLow() == 0xbeefdead);
   CPPUNIT_ASSERT(db.timeHigh() == 0xdeadbeef);
   CPPUNIT_ASSERT(db.value() == 0xdeadbeefbeefdead);
}

void testTimestamp::comparisonTest()
{
   const Timestamp small(1);
   const Timestamp med(2);

   CPPUNIT_ASSERT(small < med);
   CPPUNIT_ASSERT(small <= med);
   CPPUNIT_ASSERT(!(small == med));
   CPPUNIT_ASSERT(small != med);
   CPPUNIT_ASSERT(!(small > med));
   CPPUNIT_ASSERT(!(small >= med));

}
