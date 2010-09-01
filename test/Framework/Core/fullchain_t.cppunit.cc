/*
 *  full_chain_test.cc
 *  EDMProto
 *
 *  Created by Chris Jones on 4/3/05.
 *  Changed by Viji Sundararajan on 29-Jun-05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */
#include <iostream>
#include "art/Framework/Core/EventSetup.h"
#include "art/Framework/Core/EventSetupProvider.h"
#include "art/Framework/Core/IOVSyncValue.h"
#include "art/Framework/Core/HCMethods.icc"
#include "art/Framework/Core/recordGetImplementation.icc"
#include "art/Framework/Core/ESHandle.h"
#include "test/Framework/Core/DummyRecord.h"
#include "test/Framework/Core/DummyData.h"
#include "test/Framework/Core/DummyFinder.h"
#include "test/Framework/Core/DummyProxyProvider.h"
#include <cppunit/extensions/HelperMacros.h>

using namespace edm;
using namespace edm::eventsetup;
using namespace edm::eventsetup::test;

class testfullChain: public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(testfullChain);

CPPUNIT_TEST(getfromDataproxyproviderTest);

CPPUNIT_TEST_SUITE_END();
public:
  void setUp(){}
  void tearDown(){}

  void getfromDataproxyproviderTest();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testfullChain);

void testfullChain::getfromDataproxyproviderTest()
{
   eventsetup::EventSetupProvider provider;

   boost::shared_ptr<DataProxyProvider> pProxyProv(new DummyProxyProvider);
   provider.add(pProxyProv);

   boost::shared_ptr<DummyFinder> pFinder(new DummyFinder);
   provider.add(boost::shared_ptr<EventSetupRecordIntervalFinder>(pFinder));

   const Timestamp time_1(1);
   const IOVSyncValue sync_1(time_1);
   pFinder->setInterval(ValidityInterval(sync_1, IOVSyncValue(Timestamp(5))));
   for(unsigned int iTime=1; iTime != 6; ++iTime) {
      const Timestamp time(iTime);
      EventSetup const& eventSetup = provider.eventSetupForInstance(IOVSyncValue(time));
      ESHandle<DummyData> pDummy;
      eventSetup.get<DummyRecord>().get(pDummy);
      CPPUNIT_ASSERT(0 != &(*pDummy));

      eventSetup.getData(pDummy);

      CPPUNIT_ASSERT(0 != &(*pDummy));
   }
}
