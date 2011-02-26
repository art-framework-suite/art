
#include <iostream>

#include "art/Framework/Core/Actions.h"
#include "art/Framework/Core/CurrentProcessingContext.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ModuleFactory.h"
#include "art/Framework/Core/WorkerParams.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Utilities/GetPassID.h"
#include "art/Version/GetReleaseVersion.h"
#include "fhiclcpp/ParameterSet.h"

#include <cppunit/extensions/HelperMacros.h>

using namespace art;
using fhicl::ParameterSet;


// ----------------------------------------------
class testmaker2: public CppUnit::TestFixture
{
CPPUNIT_TEST_SUITE(testmaker2);
CPPUNIT_TEST(maker2Test);
CPPUNIT_TEST_SUITE_END();
public:
  void setUp(){}
  void tearDown(){}
  void maker2Test();
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testmaker2);

void testmaker2::maker2Test()
//int main()
{
  ParameterSet p1;
  p1.put("module_type",std::string("TestMod") );
  p1.put("module_label",std::string("t1") );

  ParameterSet p2;
  p2.put("module_type",std::string("TestMod") );
  p2.put("module_label",std::string("t2") );

  art::ActionTable table;

  art::ProductRegistry preg;
  art::WorkerParams params1(p1, p1, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());
  art::WorkerParams params2(p2, p2, preg, table, "PROD", art::getReleaseVersion(), art::getPassID());

  ModuleDescription m;
  std::auto_ptr<Worker> w1 = ModuleFactory::makeWorker(params1,m);
  std::auto_ptr<Worker> w2 = ModuleFactory::makeWorker(params2,m);

//  return 0;
}
