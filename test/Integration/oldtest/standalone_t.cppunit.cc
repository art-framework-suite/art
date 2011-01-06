/*----------------------------------------------------------------------

Test of a non fw executable

Note that the commented out lines are what is necessary to
setup the MessageLogger in this test. Note that tests like
this will hang after 1000 messages are sent to the MessageLogger
if the MessageLogger is not runnning.

----------------------------------------------------------------------*/

#include "art/Framework/PluginManager/ProblemTracker.h"
#include "art/Framework/Core/EventProcessor.h"

#include <cppunit/extensions/HelperMacros.h>

#include <memory>
#include <string>

class testStandalone: public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(testStandalone);
  CPPUNIT_TEST(writeFile);
  CPPUNIT_TEST(readFile);
  CPPUNIT_TEST_SUITE_END();


 public:

  void setUp()
  {
    m_handler = std::auto_ptr<art::AssertHandler>(new art::AssertHandler());
  }

  void tearDown(){
    m_handler.reset();
  }

  void writeFile();
  void readFile();

 private:

  std::auto_ptr<art::AssertHandler> m_handler;
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testStandalone);


void testStandalone::writeFile()
{
  std::string configuration("import FWCore.ParameterSet.Config as cms\n"
                            "process = cms.Process('TEST')\n"
                            "process.maxEvents = cms.untracked.PSet(\n"
                            "    input = cms.untracked.int32(5)\n"
                            ")\n"
                            "process.source = cms.Source('EmptySource')\n"
                            "process.JobReportService = cms.Service('JobReportService')\n"
                            "process.InitRootHandlers = cms.Service('InitRootHandlers')\n"
			    // "process.MessageLogger = cms.Service('MessageLogger')\n"
                            "process.m1 = cms.EDProducer('IntProducer',\n"
                            "    ivalue = cms.int32(11)\n"
                            ")\n"
                            "process.out = cms.OutputModule('RootOutputModule',\n"
                            "    fileName = cms.untracked.string('testStandalone.root')\n"
                            ")\n"
                            "process.p = cms.Path(process.m1)\n"
                            "process.e = cms.EndPath(process.out)\n");

  art::EventProcessor proc(configuration, true);
  proc.beginJob();
  proc.run();
  proc.endJob();
}

void testStandalone::readFile()
{
  std::string configuration("import FWCore.ParameterSet.Config as cms\n"
                            "process = cms.Process('TEST1')\n"
                            "process.source = cms.Source('RootSource',\n"
                            "    fileNames = cms.untracked.vstring('file:testStandalone.root')\n"
                            ")\n"
                            "process.InitRootHandlers = cms.Service('InitRootHandlers')\n"
                            "process.JobReportService = cms.Service('JobReportService')\n"
			    // "process.MessageLogger = cms.Service('MessageLogger')\n"
                           );

  art::EventProcessor proc(configuration, true);
  proc.beginJob();
  proc.run();
  proc.endJob();
}
