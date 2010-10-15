/*
 *  makepset_t.cc
 *  EDMProto
 *
 *  Created by Chris Jones on 5/18/05.
 *  Changed by Viji Sundararajan on 11-Jul-05.
 *
 *
 */

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

#include <iostream>

#include <stdlib.h> // for setenv; <cstdlib> is likely to fail

#include "cppunit/extensions/HelperMacros.h"
#include "boost/lambda/lambda.hpp"

#include "art/Utilities/EDMException.h"
#include "art/Utilities/Algorithms.h"
#include "art/ParameterSet/ProcessDesc.h"
#include "art/ParameterSet/PythonProcessDesc.h"



class testmakepset: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testmakepset);
  CPPUNIT_TEST(typesTest);
  CPPUNIT_TEST(secsourceTest);
  CPPUNIT_TEST(usingBlockTest);
  CPPUNIT_TEST(fileinpathTest);
  CPPUNIT_TEST_SUITE_END();

 public:
  void setUp(){}
  void tearDown(){}
  void typesTest();
  void secsourceTest();
  void usingBlockTest();
  void fileinpathTest();

 private:
  void secsourceAux();
  void usingBlockAux();
  void fileinpathAux();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testmakepset);


void testmakepset::secsourceTest()
{
  try { this->secsourceAux(); }
  catch (artZ::Exception& x) {
    std::cerr << "testmakepset::secsourceTest() caught a artZ::Exception\n";
    std::cerr << x.what() << '\n';
    throw;
  }
  catch (std::exception& x) {
    std::cerr << "testmakepset::secsourceTest() caught a std::exception\n";
    std::cerr << x.what() << '\n';
    throw;
  }
  catch (...) {
    std::cerr << "testmakepset::secsourceTest() caught an unidentified exception\n";
    throw;
  }
}

void testmakepset::secsourceAux()
{
  const char* kTest =
  "import FWCore.ParameterSet.Config as cms\n"
  "process = cms.Process('PROD')\n"
  "process.maxEvents = cms.untracked.PSet(\n"
  "    input = cms.untracked.int32(2)\n"
  ")\n"
  "process.source = cms.Source('PoolSource',\n"
  "    fileNames = cms.untracked.vstring('file:main.root')\n"
  ")\n"
  "process.out = cms.OutputModule('PoolOutputModule',\n"
  "    fileName = cms.string('file:CumHits.root')\n"
  ")\n"
  "process.mix = cms.EDFilter('MixingModule',\n"
  "    input = cms.SecSource('PoolSource',\n"
  "        fileNames = cms.untracked.vstring('file:pileup.root')\n"
  "    ),\n"
  "    max_bunch = cms.int32(3),\n"
  "    average_number = cms.double(14.3),\n"
  "    min_bunch = cms.int32(-5),\n"
  "    type = cms.string('fixed')\n"
  ")\n"
  "process.p = cms.Path(process.mix)\n"
  "process.ep = cms.EndPath(process.out)\n";

  std::string config(kTest);

  // Create the ParameterSet object from this configuration string.
  PythonProcessDesc builder(config);
  boost::shared_ptr<art::ParameterSet> ps = builder.processDesc()->getProcessPSet();

  CPPUNIT_ASSERT(0 != ps.get());

  // Make sure this ParameterSet object has the right contents
  art::ParameterSet mixingModuleParams = ps->getParameter<art::ParameterSet>("mix");
  art::ParameterSet secondarySourceParams = mixingModuleParams.getParameter<art::ParameterSet>("input");
  CPPUNIT_ASSERT(secondarySourceParams.getParameter<std::string>("@module_type") == "PoolSource");
  CPPUNIT_ASSERT(secondarySourceParams.getParameter<std::string>("@module_label") == "input");
  CPPUNIT_ASSERT(secondarySourceParams.getUntrackedParameter<std::vector<std::string> >("fileNames")[0] == "file:pileup.root");
}

void testmakepset::usingBlockTest()
{
  try { this->usingBlockAux(); }
  catch (artZ::Exception& x) {
    std::cerr << "testmakepset::usingBlockTest() caught a artZ::Exception\n";
    std::cerr << x.what() << '\n';
    throw;
  }
  catch (...) {
    std::cerr << "testmakepset::usingBlockTest() caught an unidentified exception\n";
    throw;
  }
}

void testmakepset::usingBlockAux()
{
  const char* kTest =
  "import FWCore.ParameterSet.Config as cms\n"
  "process = cms.Process('PROD')\n"
  "process.maxEvents = cms.untracked.PSet(\n"
  "    input = cms.untracked.int32(2)\n"
  ")\n"
  "process.source = cms.Source('PoolSource',\n"
  "    fileNames = cms.untracked.vstring('file:main.root')\n"
  ")\n"
  "process.b = cms.PSet(\n"
  "    s = cms.string('original'),\n"
  "    r = cms.double(1.5)\n"
  ")\n"
  "process.m1 = cms.EDFilter('Class1',\n"
  "    process.b,\n"
  "    i = cms.int32(1)\n"
  ")\n"
  "process.m2 = cms.EDFilter('Class2',\n"
  "    process.b,\n"
  "    i = cms.int32(2),\n"
  "    j = cms.int32(3),\n"
  "    u = cms.uint64(1011),\n"
  "    l = cms.int64(101010)\n"
  ")\n";


  std::string config(kTest);
  // Create the ParameterSet object from this configuration string.
  PythonProcessDesc builder(config);
  boost::shared_ptr<art::ParameterSet> ps = builder.processDesc()->getProcessPSet();

  CPPUNIT_ASSERT(0 != ps.get());

  // Make sure this ParameterSet object has the right contents
  art::ParameterSet m1Params = ps->getParameter<art::ParameterSet>("m1");
  art::ParameterSet m2Params = ps->getParameter<art::ParameterSet>("m2");
  CPPUNIT_ASSERT(m1Params.getParameter<int>("i") == 1);
  CPPUNIT_ASSERT(m2Params.getParameter<int>("i") == 2);
  CPPUNIT_ASSERT(m2Params.getParameter<int>("j") == 3);
  CPPUNIT_ASSERT(m2Params.getParameter<boost::int64_t>("l") == 101010);
  CPPUNIT_ASSERT(m2Params.getParameter<boost::uint64_t>("u") == 1011);

  CPPUNIT_ASSERT(m1Params.getParameter<std::string>("s") == "original");
  CPPUNIT_ASSERT(m2Params.getParameter<std::string>("s") == "original");

  CPPUNIT_ASSERT(m1Params.getParameter<double>("r") == 1.5);
  CPPUNIT_ASSERT(m2Params.getParameter<double>("r") == 1.5);
}

void testmakepset::fileinpathTest()
{
  try { this->fileinpathAux(); }
  catch (artZ::Exception& x) {
    std::cerr << "testmakepset::fileinpathTest() caught a artZ::Exception\n";
    std::cerr << x.what() << '\n';
    throw;
  }
  catch (...) {
    std::cerr << "testmakepset::fileinpathTest() caught an unidentified exception\n";
    throw;
  }
}

void testmakepset::fileinpathAux()
{
  const char* kTest =
  "import FWCore.ParameterSet.Config as cms\n"
  "process = cms.Process('PROD')\n"
  "process.main = cms.PSet(\n"
  "    topo = cms.FileInPath('Geometry/TrackerSimData/data/trackersens.xml'),\n"
  "    fip = cms.FileInPath('FWCore/ParameterSet/python/Config.py'),\n"
  "    ufip = cms.untracked.FileInPath('FWCore/ParameterSet/python/Types.py'),\n"
  "    extraneous = cms.int32(12)\n"
  ")\n"
  "process.source = cms.Source('DummySource')\n";

  std::string config(kTest);

  // Create the ParameterSet object from this configuration string.
  PythonProcessDesc builder(config);
  boost::shared_ptr<art::ParameterSet> ps = builder.processDesc()->getProcessPSet();
  CPPUNIT_ASSERT(0 != ps.get());

  art::ParameterSet innerps = ps->getParameter<art::ParameterSet>("main");
  art::FileInPath fip  = innerps.getParameter<art::FileInPath>("fip");
  art::FileInPath ufip = innerps.getUntrackedParameter<art::FileInPath>("ufip");
  CPPUNIT_ASSERT( innerps.existsAs<int>("extraneous") );
  CPPUNIT_ASSERT( !innerps.existsAs<int>("absent") );
  CPPUNIT_ASSERT( fip.isLocal() == true );
  CPPUNIT_ASSERT( fip.relativePath()  == "FWCore/ParameterSet/python/Config.py" );
  CPPUNIT_ASSERT( ufip.relativePath() == "FWCore/ParameterSet/python/Types.py" );
  std::string fullpath = fip.fullPath();
  std::cerr << "fullPath is: " << fip.fullPath() << std::endl;
  std::cerr << "copy of fullPath is: " << fullpath << std::endl;

  CPPUNIT_ASSERT( !fullpath.empty() );

  std::string tmpout = fullpath.substr(0, fullpath.find("FWCore/ParameterSet/python/Config.py")) + "tmp.py";

  art::FileInPath topo = innerps.getParameter<art::FileInPath>("topo");
  CPPUNIT_ASSERT( topo.isLocal() == false );
  CPPUNIT_ASSERT( topo.relativePath() == "Geometry/TrackerSimData/data/trackersens.xml" );
  fullpath = topo.fullPath();
  CPPUNIT_ASSERT( !fullpath.empty() );

  std::vector<art::FileInPath> v(1);
  CPPUNIT_ASSERT ( innerps.getAllFileInPaths(v) == 3 );

  CPPUNIT_ASSERT( v.size() == 4 );
  CPPUNIT_ASSERT( std::count(v.begin(), v.end(), fip) == 1 );
  CPPUNIT_ASSERT( std::count(v.begin(), v.end(), topo) == 1 );

  art::ParameterSet empty;
  v.clear();
  CPPUNIT_ASSERT(  empty.getAllFileInPaths(v) == 0 );
  CPPUNIT_ASSERT( v.empty() );

  // This last test checks that a FileInPath parameter can be read
  // successfully even if the associated file no longer exists.
  std::ofstream out(tmpout.c_str());
  CPPUNIT_ASSERT(!(!out));

  const char* kTest2 =
  "import FWCore.ParameterSet.Config as cms\n"
  "process = cms.Process('PROD')\n"
  "process.main = cms.PSet(\n"
  "    fip2 = cms.FileInPath('tmp.py')\n"
  ")\n"
  "process.source = cms.Source('DummySource')\n";

  std::string config2(kTest2);
  // Create the ParameterSet object from this configuration string.
  PythonProcessDesc builder2(config2);
  unlink(tmpout.c_str());
  boost::shared_ptr<art::ParameterSet> ps2 = builder2.processDesc()->getProcessPSet();

  CPPUNIT_ASSERT(0 != ps2.get());

  art::ParameterSet innerps2 = ps2->getParameter<art::ParameterSet>("main");
  art::FileInPath fip2 = innerps2.getParameter<art::FileInPath>("fip2");
  CPPUNIT_ASSERT( fip2.isLocal() == true );
  CPPUNIT_ASSERT( fip2.relativePath() == "tmp.py" );
  std::string fullpath2 = fip2.fullPath();
  std::cerr << "fullPath is: " << fip2.fullPath() << std::endl;
  std::cerr << "copy of fullPath is: " << fullpath2 << std::endl;
  CPPUNIT_ASSERT( !fullpath2.empty() );
}

void testmakepset::typesTest()
{
   //vbool vb = {true, false};
   const char* kTest =
  "import FWCore.ParameterSet.Config as cms\n"
  "process = cms.Process('t')\n"
  "process.p = cms.PSet(\n"
  "    input2 = cms.InputTag('Label2','Instance2'),\n"
  "    sb3 = cms.string('    '),\n"
  "    input1 = cms.InputTag('Label1','Instance1'),\n"
  "    input6 = cms.InputTag('source'),\n"
  "    #justasbig = cms.double(inf),\n"
  "    input4 = cms.InputTag('Label4','Instance4','Process4'),\n"
  "    input3 = cms.untracked.InputTag('Label3','Instance3'),\n"
  "    h2 = cms.uint32(255),\n"
  "    vi = cms.vint32(1, -2),\n"
  "    input8 = cms.string('deprecatedString:tag'),\n"
  "    h1 = cms.int32(74),\n"
  "    vs = cms.vstring('','1', \n"
  "        '2', \n"
  "        'a'),\n"
  "    sb2 = cms.string(''),\n"
  "    input7 = cms.InputTag('source','sink'),\n"
  "    ps = cms.PSet(\n"
  "        b2 = cms.untracked.bool(True)\n"
  "    ),\n"
  "    input5 = cms.InputTag('Label5','','Process5'),\n"
  "    h3 = cms.untracked.uint32(3487559679),\n"
  "    input = cms.InputTag('Label'),\n"
  "    vps = cms.VPSet(cms.PSet(\n"
  "        b3 = cms.bool(False)\n"
  "    )),\n"
  "    #indebt = cms.double(-inf),\n"
  "    #big = cms.double(inf),\n"
  "    vinput = cms.VInputTag(cms.InputTag('l1','i1'), cms.InputTag('l2'), cms.InputTag('l3','i3','p3'), cms.InputTag('l4','','p4'), cms.InputTag('source'), \n"
  "        cms.InputTag('source','sink')),\n"
  "    ui = cms.uint32(1),\n"
  "    eventID = cms.EventID(1, 1),\n"
  "    b = cms.untracked.bool(True),\n"
  "    d = cms.double(1.0),\n"
  "    i = cms.int32(1),\n"
  "    vui = cms.vuint32(1, 2, 1, 255),\n"
  "    s = cms.string('this string'),\n"
  "    sb1 = cms.string(''),\n"
  "    vEventID = cms.VEventID('1:1', '2:2','3:3'),\n"
  "    subRun = cms.SubRunID(55, 65),\n"
  "    vsubRuns = cms.VSubRunID('75:85', '95:105')\n"
  ")\n"

     ;

   std::string config2(kTest);
   // Create the ParameterSet object from this configuration string.
   PythonProcessDesc builder2(config2);
   boost::shared_ptr<art::ParameterSet> ps2 = builder2.processDesc()->getProcessPSet();
   art::ParameterSet test = ps2->getParameter<art::ParameterSet>("p");



   CPPUNIT_ASSERT(1 == test.getParameter<int>("i"));
   CPPUNIT_ASSERT(test.retrieve("i").isTracked());
   CPPUNIT_ASSERT(1 == test.getParameter<unsigned int>("ui"));
   CPPUNIT_ASSERT(1 == test.getParameter<double>("d"));
   //CPPUNIT_ASSERT(100000. < test.getParameter<double>("big"));
   //CPPUNIT_ASSERT(100000. < test.getParameter<double>("justasbig"));
   //CPPUNIT_ASSERT(-1000000. > test.getParameter<double>("indebt"));

   // test hex numbers
   CPPUNIT_ASSERT(74 == test.getParameter<int>("h1"));
   CPPUNIT_ASSERT(255 == test.getParameter<unsigned int>("h2"));
   CPPUNIT_ASSERT(3487559679U == test.getUntrackedParameter<unsigned int>("h3"));

   //std::cout << test.getParameter<std::string>("s") << std::endl;
   CPPUNIT_ASSERT("this string" == test.getParameter<std::string>("s"));
   //std::cout <<"blank string using single quotes returns \""<<test.getParameter<std::string>("sb1")<<"\""<<std::endl;
   //std::cout <<"blank string using double quotes returns \""<<test.getParameter<std::string>("sb2")<<"\""<<std::endl;
   CPPUNIT_ASSERT("" == test.getParameter<std::string>("sb1"));
   CPPUNIT_ASSERT("" == test.getParameter<std::string>("sb2"));
   CPPUNIT_ASSERT(4  == test.getParameter<std::string>("sb3").size());
   std::vector<std::string> vs = test.getParameter<std::vector<std::string> >("vs");
   int vssize = vs.size();
   //FIXME doesn't do spaces right
   //CPPUNIT_ASSERT(4 == vssize);
   //CPPUNIT_ASSERT(vssize && "" == vs[0]);
   //CPPUNIT_ASSERT(vssize >1 && "1" == vs[1]);
   //CPPUNIT_ASSERT(vssize >1 && "a" == vs[3]);
   //std::cout <<"\""<<test.getParameter<std::vector<std::string> >("vs")[0]<<"\" \""<<test.getParameter<std::vector<std::string> >("vs")[1]<<"\" \""
   //<<test.getParameter<std::vector<std::string> >("vs")[2]<<"\""<<std::endl;

   static const unsigned int vuia[] = {1,2,1,255};
   static const std::vector<unsigned int> vui(vuia, vuia+sizeof(vuia)/sizeof(unsigned int));
   CPPUNIT_ASSERT(vui == test.getParameter<std::vector<unsigned int> >("vui"));

   static const  int via[] = {1,-2};
   static const std::vector<int> vi(via, via+sizeof(vuia)/sizeof(unsigned int));
   test.getParameter<std::vector<int> >("vi");
   CPPUNIT_ASSERT(true == test.getUntrackedParameter<bool>("b", false));
   CPPUNIT_ASSERT(test.retrieve("vi").isTracked());
   //test.getParameter<std::vector<bool> >("vb");
   art::ParameterSet ps = test.getParameter<art::ParameterSet>("ps");
   CPPUNIT_ASSERT(true == ps.getUntrackedParameter<bool>("b2", false));
   std::vector<art::ParameterSet> vps = test.getParameter<std::vector<art::ParameterSet> >("vps");
   CPPUNIT_ASSERT(1 == vps.size());
   CPPUNIT_ASSERT(false == vps.front().getParameter<bool>("b3"));

   // InputTag
   art::InputTag inputProduct  = test.getParameter<art::InputTag>("input");
   art::InputTag inputProduct1 = test.getParameter<art::InputTag>("input1");
   art::InputTag inputProduct2 = test.getParameter<art::InputTag>("input2");
   art::InputTag inputProduct3 = test.getUntrackedParameter<art::InputTag>("input3");
   art::InputTag inputProduct4 = test.getParameter<art::InputTag>("input4");
   art::InputTag inputProduct5 = test.getParameter<art::InputTag>("input5");
   art::InputTag inputProduct6 = test.getParameter<art::InputTag>("input6");
   art::InputTag inputProduct7 = test.getParameter<art::InputTag>("input7");
   art::InputTag inputProduct8 = test.getParameter<art::InputTag>("input8");

   //art::OutputTag outputProduct = test.getParameter<art::OutputTag>("output");

   CPPUNIT_ASSERT("Label"    == inputProduct.label());
   CPPUNIT_ASSERT("Label1"    == inputProduct1.label());
   CPPUNIT_ASSERT("Label2"    == inputProduct2.label());
   CPPUNIT_ASSERT("Instance2" == inputProduct2.instance());
   CPPUNIT_ASSERT("Label3"    == inputProduct3.label());
   CPPUNIT_ASSERT("Instance3" == inputProduct3.instance());
   CPPUNIT_ASSERT("Label4" == inputProduct4.label());
   CPPUNIT_ASSERT("Instance4" == inputProduct4.instance());
   CPPUNIT_ASSERT("Process4" == inputProduct4.process());
   CPPUNIT_ASSERT("Label5" == inputProduct5.label());
   CPPUNIT_ASSERT("" == inputProduct5.instance());
   CPPUNIT_ASSERT("Process5" == inputProduct5.process());
   CPPUNIT_ASSERT("source" == inputProduct6.label());
   CPPUNIT_ASSERT("source" == inputProduct7.label());
   CPPUNIT_ASSERT("deprecatedString" == inputProduct8.label());


   // vector of InputTags

   std::vector<art::InputTag> vtags = test.getParameter<std::vector<art::InputTag> >("vinput");
   CPPUNIT_ASSERT("l1" == vtags[0].label());
   CPPUNIT_ASSERT("i1" == vtags[0].instance());
   CPPUNIT_ASSERT("l2" == vtags[1].label());
   CPPUNIT_ASSERT("l3" == vtags[2].label());
   CPPUNIT_ASSERT("i3" == vtags[2].instance());
   CPPUNIT_ASSERT("p3" == vtags[2].process());
   CPPUNIT_ASSERT("l4" == vtags[3].label());
   CPPUNIT_ASSERT(""   == vtags[3].instance());
   CPPUNIT_ASSERT("p4" == vtags[3].process());
   CPPUNIT_ASSERT("source" == vtags[4].label());
   CPPUNIT_ASSERT("source" == vtags[5].label());

   art::EventID eventID = test.getParameter<art::EventID>("eventID");
   std::vector<art::EventID> vEventID = test.getParameter<std::vector<art::EventID> >("vEventID");
   CPPUNIT_ASSERT(1 == eventID.run());
   CPPUNIT_ASSERT(1 == eventID.event());
   CPPUNIT_ASSERT(1 == vEventID[0].run());
   CPPUNIT_ASSERT(1 == vEventID[0].event());
   CPPUNIT_ASSERT(3 == vEventID[2].run());
   CPPUNIT_ASSERT(3 == vEventID[2].event());

   art::SubRunID subRun = test.getParameter<art::SubRunID >("subRun");
   CPPUNIT_ASSERT(55 == subRun.run());
   CPPUNIT_ASSERT(65 == subRun.subRun());
   std::vector<art::SubRunID> vsubRuns = test.getParameter<std::vector<art::SubRunID> >("vsubRuns");
   CPPUNIT_ASSERT(vsubRuns.size() == 2);
   CPPUNIT_ASSERT(vsubRuns[0].run() == 75);
   CPPUNIT_ASSERT(vsubRuns[0].subRun() == 85);
   CPPUNIT_ASSERT(vsubRuns[1].run() == 95);
   CPPUNIT_ASSERT(vsubRuns[1].subRun() == 105);


   //CPPUNIT_ASSERT("Label2" == outputProduct.label());
   //CPPUNIT_ASSERT(""       == outputProduct.instance());
   //CPPUNIT_ASSERT("Alias2" == outputProduct.alias());
   //BOOST_CHECK_THROW(makePSet(*nodeList), std::runtime_error);
}

