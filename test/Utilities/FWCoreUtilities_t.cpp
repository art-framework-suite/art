/*----------------------------------------------------------------------

Test program for edm::TypeIDBase class.
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
  edm::CPUTimer timer;
  CPPUNIT_ASSERT(timer.realTime() == 0.0);
  CPPUNIT_ASSERT(timer.cpuTime()==0.0);

  timer.start();
  sleep(2);
  timer.stop();
  std::cerr <<"real "<<timer.realTime()<<" cpu "<<timer.cpuTime()<< std::endl;
  CPPUNIT_ASSERT(timer.realTime() > 2.0);
  CPPUNIT_ASSERT(timer.cpuTime()+2.0 <= timer.realTime());

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

/*----------------------------------------------------------------------

Test program for edm::ExtensionCord class.
Created by Chris Jones on 22-09-2006


 ----------------------------------------------------------------------*/

#include <cppunit/extensions/HelperMacros.h>
#include "art/Utilities/ExtensionCord.h"
#include "art/Utilities/SimpleOutlet.h"

class testExtensionCord: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testExtensionCord);

  CPPUNIT_TEST(unpluggedTest);
  CPPUNIT_TEST(pluggedTest);
  CPPUNIT_TEST(copyTest);

  CPPUNIT_TEST_SUITE_END();
public:
  void setUp(){}
  void tearDown(){}

  void unpluggedTest();
  void pluggedTest();
  void copyTest();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testExtensionCord);


void testExtensionCord::unpluggedTest()
{
  edm::ExtensionCord<int> dangling;
  CPPUNIT_ASSERT(!dangling.connected());

  CPPUNIT_ASSERT_THROW(*dangling, cms::Exception);
  CPPUNIT_ASSERT_THROW(dangling.operator->(), cms::Exception);
}

void testExtensionCord::pluggedTest()
{
  edm::ExtensionCord<int> cord;

  {
    int value(1);
    edm::ValueHolderECGetter<int> getter(value);

    edm::SimpleOutlet<int> outlet( getter, cord );

    CPPUNIT_ASSERT( 1 == *cord);
  }
  CPPUNIT_ASSERT(!cord.connected());
}

void testExtensionCord::copyTest()
{
  edm::ExtensionCord<int> cord1;
  edm::ExtensionCord<int> cord2(cord1);

  {
    int value(1);
    edm::ValueHolderECGetter<int> getter(value);

    edm::SimpleOutlet<int> outlet(getter, cord1 );

    CPPUNIT_ASSERT( 1 == *cord2);
  }
  CPPUNIT_ASSERT(!cord2.connected());
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

using namespace edm;

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
  classToFriendly.insert( Values("bar::Foo","barFoo") );
  classToFriendly.insert( Values("std::vector<Foo>","Foos") );
  classToFriendly.insert( Values("std::vector<bar::Foo>","barFoos") );
  classToFriendly.insert( Values("V<A,B>","ABV") );
  classToFriendly.insert( Values("edm::ExtCollection<std::vector<reco::SuperCluster>,reco::SuperClusterRefProds>","recoSuperClustersrecoSuperClusterRefProdsedmExtCollection") );
  classToFriendly.insert( Values("edm::SortedCollection<EcalUncalibratedRecHit,edm::StrictWeakOrdering<EcalUncalibratedRecHit> >","EcalUncalibratedRecHitsSorted") );
  classToFriendly.insert( Values("edm::OwnVector<aod::Candidate,edm::ClonePolicy<aod::Candidate> >","aodCandidatesOwned") );
  classToFriendly.insert( Values("edm::OwnVector<Foo,edm::ClonePolicy<Foo> >","FoosOwned") );
  classToFriendly.insert( Values("edm::OwnVector<My<int>, edm::ClonePolicy<My<int> > >","intMysOwned") );
  classToFriendly.insert( Values("std::vector<edm::OwnVector<My<int>, edm::ClonePolicy<My<int> > > >","intMysOwneds") );
  classToFriendly.insert( Values("edm::Wrapper<MuonDigiCollection<CSCDetId,CSCALCTDigi> >","CSCDetIdCSCALCTDigiMuonDigiCollection") );
  classToFriendly.insert( Values("edm::AssociationMap<edm::OneToMany<std::vector<CaloJet>,std::vector<reco::Track>,unsigned int> >","CaloJetsToManyrecoTracksAssociation") );
  classToFriendly.insert( Values("edm::AssociationMap<edm::OneToOne<std::vector<reco::Track>,std::vector<reco::TrackInfo>,unsigned int> >","recoTracksToOnerecoTrackInfosAssociation") );
  classToFriendly.insert( Values("edm::AssociationMap<edm::OneToValue<std::vector<reco::Electron>,float,unsigned int> >",
                                 "recoElectronsToValuefloatAssociation"));
  classToFriendly.insert( Values("edm::AssociationMap<edm::OneToManyWithQuality<edm::OwnVector<reco::Candidate,edm::ClonePolicy<reco::Candidate> >,edm::OwnVector<reco::Candidate,edm::ClonePolicy<reco::Candidate> >,double,unsigned int> >",
                                 "recoCandidatesOwnedToManyrecoCandidatesOwnedWithQuantitydoubleAssociation"));
  classToFriendly.insert( Values("edm::AssociationVector<edm::RefProd<std::vector<reco::CaloJet> >,std::vector<int>,edm::Ref<std::vector<reco::CaloJet>,reco::CaloJet,edm::refhelper::FindUsingAdvance<std::vector<reco::CaloJet>,reco::CaloJet> >,unsigned int,edm::helper::AssociationIdenticalKeyReference>",
                                 "recoCaloJetsedmRefProdTointsAssociationVector") );
  classToFriendly.insert( Values("edm::AssociationVector<edm::RefProd<edm::OwnVector<reco::Candidate,edm::ClonePolicy<reco::Candidate> > >,std::vector<double>,edm::Ref<edm::OwnVector<reco::Candidate,edm::ClonePolicy<reco::Candidate> >,reco::Candidate,edm::refhelper::FindUsingAdvance<edm::OwnVector<reco::Candidate,edm::ClonePolicy<reco::Candidate> >,reco::Candidate> >,unsigned int,edm::helper::AssociationIdenticalKeyReference>",
                                 "recoCandidatesOwnededmRefProdTodoublesAssociationVector") );
  classToFriendly.insert( Values("edm::AssociationMap<edm::OneToOne<std::vector<reco::Track>,std::vector<std::pair<double,double> >,unsigned int> >",
                                 "recoTracksToOnedoubledoublestdpairsAssociation"));
  classToFriendly.insert( Values("edm::AssociationMap<edm::OneToOne<std::vector<reco::Track>,std::vector<std::pair<Point3DBase<float,GlobalTag>,GlobalErrorBase<double,ErrorMatrixTag> > >,unsigned int> >",
                                 "recoTracksToOnefloatGlobalTagPoint3DBasedoubleErrorMatrixTagGlobalErrorBasestdpairsAssociation"));
  classToFriendly.insert( Values("A<B<C>, D<E> >","CBEDA"));
  classToFriendly.insert( Values("A<B<C<D> > >","DCBA"));
  classToFriendly.insert( Values("A<B<C,D>, E<F> >","CDBFEA"));
  classToFriendly.insert( Values("Aa<Bb<Cc>, Dd<Ee> >","CcBbEeDdAa"));
  classToFriendly.insert( Values("Aa<Bb<Cc<Dd> > >","DdCcBbAa"));
  classToFriendly.insert( Values("Aa<Bb<Cc,Dd>, Ee<Ff> >","CcDdBbFfEeAa"));
  classToFriendly.insert( Values("Aa<Bb<Cc,Dd>, Ee<Ff,Gg> >","CcDdBbFfGgEeAa"));
  classToFriendly.insert( Values("edm::RangeMap<DetId,edm::OwnVector<SiPixelRecHit,edm::ClonePolicy<SiPixelRecHit> >,edm::ClonePolicy<SiPixelRecHit> >","DetIdSiPixelRecHitsOwnedRangeMap"));
  classToFriendly.insert( Values("std::vector<edm::RangeMap<DetId,edm::OwnVector<SiPixelRecHit,edm::ClonePolicy<SiPixelRecHit> >,edm::ClonePolicy<SiPixelRecHit> > >","DetIdSiPixelRecHitsOwnedRangeMaps"));
  classToFriendly.insert( Values("edm::RefVector< edm::OwnVector<reco::Candidate,edm::ClonePolicy<reco::Candidate> >,reco::Candidate, edm::refhelper::FindUsingAdvance<edm::OwnVector<reco::Candidate,edm::ClonePolicy<reco::Candidate> >, reco::Candidate> >","recoCandidatesOwnedRefs"));
  classToFriendly.insert( Values("edm::RefVector< std::vector<reco::Track>, reco::Track, edm::refhelper::FindUsingAdvance<std::vector<reco::Track>, reco::Track> >","recoTracksRefs"));
  classToFriendly.insert( Values("edm::RefVector<Col, Type, edm::refhelper::FindUsingAdvance<Col, Type> >","ColTypeRefs"));
  classToFriendly.insert( Values("edm::AssociationMap<edm::OneToMany<std::vector<reco::PixelMatchGsfElectron>,edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> >,unsigned int> >",
                                 "recoPixelMatchGsfElectronsToManyEcalRecHitsSortedAssociation"));
  classToFriendly.insert( Values("edm::AssociationVector<edm::RefToBaseProd<reco::Candidate>,std::vector<double>,edm::RefToBase<reco::Candidate>,unsigned int,edm::helper::AssociationIdenticalKeyReference>",
                                 "recoCandidateedmRefToBaseProdTodoublesAssociationVector"));
  classToFriendly.insert( Values("edm::RefVector<edm::AssociationMap<edm::OneToOne<std::vector<reco::BasicCluster>,std::vector<reco::ClusterShape>,unsigned int> >,edm::helpers::KeyVal<edm::Ref<std::vector<reco::BasicCluster>,reco::BasicCluster,edm::refhelper::FindUsingAdvance<std::vector<reco::BasicCluster>,reco::BasicCluster> >,edm::Ref<std::vector<reco::ClusterShape>,reco::ClusterShape,edm::refhelper::FindUsingAdvance<std::vector<reco::ClusterShape>,reco::ClusterShape> > >,edm::AssociationMap<edm::OneToOne<std::vector<reco::BasicCluster>,std::vector<reco::ClusterShape>,unsigned int> >::Find>",
                                 "recoBasicClustersToOnerecoClusterShapesAssociationRefs"));

  for(std::map<std::string, std::string>::iterator itInfo = classToFriendly.begin(),
      itInfoEnd = classToFriendly.end();
      itInfo != itInfoEnd;
      ++itInfo) {
    //std::cout <<itInfo->first<<std::endl;
    if( itInfo->second != edm::friendlyname::friendlyName(itInfo->first) ) {
      std::cout <<"class name: '"<<itInfo->first<<"' has wrong friendly name \n"
      <<"expect: '"<<itInfo->second<<"' got: '"<<edm::friendlyname::friendlyName(itInfo->first)<<"'"<<std::endl;
      CPPUNIT_ASSERT(0 && "expected friendly name does not match actual friendly name");
    }
  }
}
/*----------------------------------------------------------------------

Test program for edm::TypeIDBase class.
Changed by Viji on 29-06-2005


 ----------------------------------------------------------------------*/

#include <cassert>
#include <iostream>
#include <string>
#include <cppunit/extensions/HelperMacros.h>
#include "art/Utilities/TypeIDBase.h"

class testTypeIDBase: public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(testTypeIDBase);

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
CPPUNIT_TEST_SUITE_REGISTRATION(testTypeIDBase);

namespace edmtest {
  struct empty { };
}

void testTypeIDBase::equalityTest()

{
  edmtest::empty e;
  edm::TypeIDBase id1(typeid(e));
  edm::TypeIDBase id2(typeid(e));

  CPPUNIT_ASSERT(!(id1 < id2));
  CPPUNIT_ASSERT(!(id2 < id1));

  std::string n1(id1.name());
  std::string n2(id2.name());

  CPPUNIT_ASSERT(n1==n2);
}

void testTypeIDBase::copyTest()
{
  edmtest::empty e;
  edm::TypeIDBase id1(typeid(e));

  edm::TypeIDBase id3=id1;
  CPPUNIT_ASSERT(!(id1 < id3));
  CPPUNIT_ASSERT(!(id3 < id1));

  std::string n1(id1.name());
  std::string n3(id3.name());
  CPPUNIT_ASSERT(n1== n3);
}
#include <Utilities/Testing/interface/CppUnit_testdriver.icpp>
/*----------------------------------------------------------------------

Test program for edm::TypeID class.
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
  edmtest::empty e;
  edm::TypeID id1(e);
  edm::TypeID id2(e);

  CPPUNIT_ASSERT(!(id1 < id2));
  CPPUNIT_ASSERT(!(id2 < id1));

  std::string n1(id1.name());
  std::string n2(id2.name());

  CPPUNIT_ASSERT(n1==n2);
}

void testTypeid::copyTest()
{
  edmtest::empty e;
  edm::TypeID id1(e);

  edm::TypeID id3=id1;
  CPPUNIT_ASSERT(!(id1 < id3));
  CPPUNIT_ASSERT(!(id3 < id1));

  std::string n1(id1.name());
  std::string n3(id3.name());
  CPPUNIT_ASSERT(n1== n3);
}
