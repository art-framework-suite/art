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
  classToFriendly.insert( Values("art::ExtCollection<std::vector<reco::SuperCluster>,reco::SuperClusterRefProds>","reco::SuperClustersreco::SuperClusterRefProdsart::ExtCollection") );
  classToFriendly.insert( Values("art::SortedCollection<EcalUncalibratedRecHit,art::StrictWeakOrdering<EcalUncalibratedRecHit> >","EcalUncalibratedRecHitsSorted") );
  classToFriendly.insert( Values("art::OwnVector<aod::Candidate,art::ClonePolicy<aod::Candidate> >","aod::CandidatesOwned") );
  classToFriendly.insert( Values("art::OwnVector<Foo,art::ClonePolicy<Foo> >","FoosOwned") );
  classToFriendly.insert( Values("art::OwnVector<My<int>, art::ClonePolicy<My<int> > >","intMysOwned") );
  classToFriendly.insert( Values("std::vector<art::OwnVector<My<int>, art::ClonePolicy<My<int> > > >","intMysOwneds") );
  classToFriendly.insert( Values("art::Wrapper<MuonDigiCollection<CSCDetId,CSCALCTDigi> >","CSCDetIdCSCALCTDigiMuonDigiCollection") );
  classToFriendly.insert( Values("art::AssociationMap<art::OneToMany<std::vector<CaloJet>,std::vector<reco::Track>,unsigned int> >","CaloJetsToManyreco::TracksAssociation") );
  classToFriendly.insert( Values("art::AssociationMap<art::OneToOne<std::vector<reco::Track>,std::vector<reco::TrackInfo>,unsigned int> >","reco::TracksToOnereco::TrackInfosAssociation") );
  classToFriendly.insert( Values("art::AssociationMap<art::OneToValue<std::vector<reco::Electron>,float,unsigned int> >",
                                 "reco::ElectronsToValuefloatAssociation"));
  classToFriendly.insert( Values("art::AssociationMap<art::OneToManyWithQuality<art::OwnVector<reco::Candidate,art::ClonePolicy<reco::Candidate> >,art::OwnVector<reco::Candidate,art::ClonePolicy<reco::Candidate> >,double,unsigned int> >",
                                 "reco::CandidatesOwnedToManyreco::CandidatesOwnedWithQuantitydoubleAssociation"));
  classToFriendly.insert( Values("art::AssociationVector<art::RefProd<std::vector<reco::CaloJet> >,std::vector<int>,art::Ref<std::vector<reco::CaloJet>,reco::CaloJet,art::refhelper::FindUsingAdvance<std::vector<reco::CaloJet>,reco::CaloJet> >,unsigned int,art::helper::AssociationIdenticalKeyReference>",
                                 "reco::CaloJetsart::RefProdTointsAssociationVector") );
  classToFriendly.insert( Values("art::AssociationVector<art::RefProd<art::OwnVector<reco::Candidate,art::ClonePolicy<reco::Candidate> > >,std::vector<double>,art::Ref<art::OwnVector<reco::Candidate,art::ClonePolicy<reco::Candidate> >,reco::Candidate,art::refhelper::FindUsingAdvance<art::OwnVector<reco::Candidate,art::ClonePolicy<reco::Candidate> >,reco::Candidate> >,unsigned int,art::helper::AssociationIdenticalKeyReference>",
                                 "reco::CandidatesOwnedart::RefProdTodoublesAssociationVector") );
  classToFriendly.insert( Values("art::AssociationMap<art::OneToOne<std::vector<reco::Track>,std::vector<std::pair<double,double> >,unsigned int> >",
                                 "reco::TracksToOnedoubledoublestd::pairsAssociation"));
  classToFriendly.insert( Values("art::AssociationMap<art::OneToOne<std::vector<reco::Track>,std::vector<std::pair<Point3DBase<float,GlobalTag>,GlobalErrorBase<double,ErrorMatrixTag> > >,unsigned int> >",
                                 "reco::TracksToOnefloatGlobalTagPoint3DBasedoubleErrorMatrixTagGlobalErrorBasestd::pairsAssociation"));
  classToFriendly.insert( Values("A<B<C>, D<E> >","CBEDA"));
  classToFriendly.insert( Values("A<B<C<D> > >","DCBA"));
  classToFriendly.insert( Values("A<B<C,D>, E<F> >","CDBFEA"));
  classToFriendly.insert( Values("Aa<Bb<Cc>, Dd<Ee> >","CcBbEeDdAa"));
  classToFriendly.insert( Values("Aa<Bb<Cc<Dd> > >","DdCcBbAa"));
  classToFriendly.insert( Values("Aa<Bb<Cc,Dd>, Ee<Ff> >","CcDdBbFfEeAa"));
  classToFriendly.insert( Values("Aa<Bb<Cc,Dd>, Ee<Ff,Gg> >","CcDdBbFfGgEeAa"));
  classToFriendly.insert( Values("art::RangeMap<DetId,art::OwnVector<SiPixelRecHit,art::ClonePolicy<SiPixelRecHit> >,art::ClonePolicy<SiPixelRecHit> >","DetIdSiPixelRecHitsOwnedRangeMap"));
  classToFriendly.insert( Values("std::vector<art::RangeMap<DetId,art::OwnVector<SiPixelRecHit,art::ClonePolicy<SiPixelRecHit> >,art::ClonePolicy<SiPixelRecHit> > >","DetIdSiPixelRecHitsOwnedRangeMaps"));
  classToFriendly.insert( Values("art::RefVector< art::OwnVector<reco::Candidate,art::ClonePolicy<reco::Candidate> >,reco::Candidate, art::refhelper::FindUsingAdvance<art::OwnVector<reco::Candidate,art::ClonePolicy<reco::Candidate> >, reco::Candidate> >","reco::CandidatesOwnedRefs"));
  classToFriendly.insert( Values("art::RefVector< std::vector<reco::Track>, reco::Track, art::refhelper::FindUsingAdvance<std::vector<reco::Track>, reco::Track> >","reco::TracksRefs"));
  classToFriendly.insert( Values("art::RefVector<Col, Type, art::refhelper::FindUsingAdvance<Col, Type> >","ColTypeRefs"));
  classToFriendly.insert( Values("art::AssociationMap<art::OneToMany<std::vector<reco::PixelMatchGsfElectron>,art::SortedCollection<EcalRecHit,art::StrictWeakOrdering<EcalRecHit> >,unsigned int> >",
                                 "reco::PixelMatchGsfElectronsToManyEcalRecHitsSortedAssociation"));
  classToFriendly.insert( Values("art::AssociationVector<art::RefToBaseProd<reco::Candidate>,std::vector<double>,art::RefToBase<reco::Candidate>,unsigned int,art::helper::AssociationIdenticalKeyReference>",
                                 "reco::Candidateart::RefToBaseProdTodoublesAssociationVector"));
  classToFriendly.insert( Values("art::RefVector<art::AssociationMap<art::OneToOne<std::vector<reco::BasicCluster>,std::vector<reco::ClusterShape>,unsigned int> >,art::helpers::KeyVal<art::Ref<std::vector<reco::BasicCluster>,reco::BasicCluster,art::refhelper::FindUsingAdvance<std::vector<reco::BasicCluster>,reco::BasicCluster> >,art::Ref<std::vector<reco::ClusterShape>,reco::ClusterShape,art::refhelper::FindUsingAdvance<std::vector<reco::ClusterShape>,reco::ClusterShape> > >,art::AssociationMap<art::OneToOne<std::vector<reco::BasicCluster>,std::vector<reco::ClusterShape>,unsigned int> >::Find>",
                                 "reco::BasicClustersToOnereco::ClusterShapesAssociationRefs"));
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
/*----------------------------------------------------------------------

Test program for art::TypeID class.
Changed by Viji on 29-06-2005


 ----------------------------------------------------------------------*/

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
