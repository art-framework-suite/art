/*----------------------------------------------------------------------

Test program for art::TypeID class.
Changed by Viji on 29-06-2005


 ----------------------------------------------------------------------*/

#include <cassert>
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/resource.h>
#include <cppunit/extensions/HelperMacros.h>
#include "art/Utilities/CPUTimer.h"

class testCPUTimer: public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(testCPUTimer);

CPPUNIT_TEST(testTiming);

CPPUNIT_TEST_SUITE_END();
public:
  void setUp(){}
  void tearDown(){}

  void testTiming();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testCPUTimer);

using std::cerr;
using std::endl;

void testCPUTimer::testTiming()

{
  art::CPUTimer timer;
  CPPUNIT_ASSERT(timer.realTime() == 0.0);
  CPPUNIT_ASSERT(timer.cpuTime()==0.0);

  timer.start();
  sleep(2);
  timer.stop();
  std::cerr <<"real "<<timer.realTime()<<" cpu "<<timer.cpuTime()<< std::endl;
  CPPUNIT_ASSERT(timer.realTime() > 2.0);
  CPPUNIT_ASSERT(timer.cpuTime()+1.5 <= timer.realTime()); // Allow for timer slop.

  timer.start();
  sleep(2);
  std::cerr <<"real "<<timer.realTime()<<" cpu "<<timer.cpuTime()<< std::endl;
  CPPUNIT_ASSERT(timer.realTime() > 4.0);
  //this should do nothing
  timer.start();
  CPPUNIT_ASSERT(timer.realTime() > 4.0);

  sleep(2);

  timer.stop();
  std::cerr <<"real "<<timer.realTime()<<" cpu "<<timer.cpuTime()<< std::endl;

  double real = timer.realTime();
  double cpu = timer.cpuTime();


  //this should do nothing
  timer.stop();
  CPPUNIT_ASSERT(timer.realTime()==real);
  CPPUNIT_ASSERT(timer.cpuTime()==cpu);

  timer.reset();
  CPPUNIT_ASSERT(timer.realTime() == 0.0);
  CPPUNIT_ASSERT(timer.cpuTime()==0.0);

  rusage theUsage;
  getrusage(RUSAGE_SELF, &theUsage) ;
  struct timeval startTime;
  startTime.tv_sec =theUsage.ru_utime.tv_sec;
  startTime.tv_usec =theUsage.ru_utime.tv_usec;

  timer.start();
  struct timeval nowTime;
  do {
    rusage theUsage2;
    getrusage(RUSAGE_SELF, &theUsage2) ;
    nowTime.tv_sec =theUsage2.ru_utime.tv_sec;
    nowTime.tv_usec =theUsage2.ru_utime.tv_usec;
  }while(nowTime.tv_sec -startTime.tv_sec +1E-6*(nowTime.tv_usec-startTime.tv_usec) <1);
  timer.stop();

  std::cerr <<"real "<<timer.realTime()<<" cpu "<<timer.cpuTime()<< std::endl;
  CPPUNIT_ASSERT(timer.realTime() >= 1.0);
  CPPUNIT_ASSERT(timer.cpuTime()>=1.0);

}
/*
 *  friendlyname_t.cppunit.cc
 *  CMSSW
 *
 *  Created by Chris Jones on 8/8/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include <cppunit/extensions/HelperMacros.h>
#include <iostream>
#include "art/Utilities/FriendlyName.h"

using namespace art;

class testfriendlyName: public CppUnit::TestFixture
{
   CPPUNIT_TEST_SUITE(testfriendlyName);

   CPPUNIT_TEST(test);

   CPPUNIT_TEST_SUITE_END();
public:
      void setUp(){}
   void tearDown(){}

   void test();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testfriendlyName);


void testfriendlyName::test()
{
  typedef std::pair<std::string,std::string> Values;
  std::map<std::string, std::string> classToFriendly;
  classToFriendly.insert( Values("Foo","Foo") );
  classToFriendly.insert( Values("bar::Foo","bar::Foo") );
  classToFriendly.insert( Values("std::vector<Foo>","Foos") );
  classToFriendly.insert( Values("std::vector<bar::Foo>","bar::Foos") );
  classToFriendly.insert( Values("V<A,B>","ABV") );
  classToFriendly.insert( Values("art::Wrapper<MuonDigiCollection<CSCDetId,CSCALCTDigi> >","CSCDetIdCSCALCTDigiMuonDigiCollection") );
  classToFriendly.insert( Values("A<B<C>, D<E> >","CBEDA"));
  classToFriendly.insert( Values("A<B<C<D> > >","DCBA"));
  classToFriendly.insert( Values("A<B<C,D>, E<F> >","CDBFEA"));
  classToFriendly.insert( Values("Aa<Bb<Cc>, Dd<Ee> >","CcBbEeDdAa"));
  classToFriendly.insert( Values("Aa<Bb<Cc<Dd> > >","DdCcBbAa"));
  classToFriendly.insert( Values("Aa<Bb<Cc,Dd>, Ee<Ff> >","CcDdBbFfEeAa"));
  classToFriendly.insert( Values("Aa<Bb<Cc,Dd>, Ee<Ff,Gg> >","CcDdBbFfGgEeAa"));
  classToFriendly.insert( Values("cet::map_vector_key","mvk") );
  classToFriendly.insert( Values("cet::map_vector<Foo>","Foomv") );

  for(std::map<std::string, std::string>::iterator itInfo = classToFriendly.begin(),
      itInfoEnd = classToFriendly.end();
      itInfo != itInfoEnd;
      ++itInfo) {
    //std::cout <<itInfo->first<<std::endl;
    if( itInfo->second != art::friendlyname::friendlyName(itInfo->first) ) {
      std::cout <<"class name: '"<<itInfo->first<<"' has wrong friendly name \n"
      <<"expect: '"<<itInfo->second<<"' got: '"<<art::friendlyname::friendlyName(itInfo->first)<<"'"<<std::endl;
      CPPUNIT_ASSERT(0 && "expected friendly name does not match actual friendly name");
    }
  }
}

#include <cassert>
#include <iostream>
#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include "art/Utilities/TypeID.h"

class testTypeID: public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(testTypeID);

CPPUNIT_TEST(equalityTest);
CPPUNIT_TEST(copyTest);

CPPUNIT_TEST_SUITE_END();
public:
  void setUp(){}
  void tearDown(){}

  void equalityTest();
  void copyTest();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testTypeID);

namespace arttest {
  struct empty { };
}

void testTypeID::equalityTest()

{
  arttest::empty e;
  art::TypeID id1(typeid(e));
  art::TypeID id2(typeid(e));

  CPPUNIT_ASSERT(!(id1 < id2));
  CPPUNIT_ASSERT(!(id2 < id1));

  std::string n1(id1.name());
  std::string n2(id2.name());

  CPPUNIT_ASSERT(n1==n2);
}

void testTypeID::copyTest()
{
  arttest::empty e;
  art::TypeID id1(typeid(e));

  art::TypeID id3=id1;
  CPPUNIT_ASSERT(!(id1 < id3));
  CPPUNIT_ASSERT(!(id3 < id1));

  std::string n1(id1.name());
  std::string n3(id3.name());
  CPPUNIT_ASSERT(n1== n3);
}
#include "test/CppUnit_testdriver.icpp"
/*----------------------------------------------------------------------

Test program for art::TypeID class.
Changed by Viji on 29-06-2005


 ----------------------------------------------------------------------*/

#include <cassert>
#include <iostream>
#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include "art/Utilities/TypeID.h"

class testTypeid: public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(testTypeid);

CPPUNIT_TEST(equalityTest);
CPPUNIT_TEST(copyTest);

CPPUNIT_TEST_SUITE_END();
public:
  void setUp(){}
  void tearDown(){}

  void equalityTest();
  void copyTest();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testTypeid);


using std::cerr;
using std::endl;

void testTypeid::equalityTest()

{
  arttest::empty e;
  art::TypeID id1(e);
  art::TypeID id2(e);

  CPPUNIT_ASSERT(!(id1 < id2));
  CPPUNIT_ASSERT(!(id2 < id1));

  std::string n1(id1.name());
  std::string n2(id2.name());

  CPPUNIT_ASSERT(n1==n2);
}

void testTypeid::copyTest()
{
  arttest::empty e;
  art::TypeID id1(e);

  art::TypeID id3=id1;
  CPPUNIT_ASSERT(!(id1 < id3));
  CPPUNIT_ASSERT(!(id3 < id1));

  std::string n1(id1.name());
  std::string n3(id3.name());
  CPPUNIT_ASSERT(n1== n3);
}
