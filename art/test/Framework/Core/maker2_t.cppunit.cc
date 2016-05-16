
#include <iostream>

#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/CurrentProcessingContext.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Framework/Core/detail/ModuleFactory.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Core/WorkerT.h"
#include "canvas/Utilities/GetPassID.h"
#include "art/Version/GetReleaseVersion.h"
#include "fhiclcpp/ParameterSet.h"

#include <cppunit/extensions/HelperMacros.h>

using namespace art;
using fhicl::ParameterSet;

namespace
{
  ModuleDescription
    createModuleDescription(WorkerParams const &p)
  {
    ParameterSet const& procParams = p.procPset_;
    ParameterSet const& conf = p.pset_;
    return ModuleDescription(conf.id(),
                             conf.get<std::string>("module_type"),
                             conf.get<std::string>("module_label"),
                             ProcessConfiguration(p.processName_,
                                                  procParams.id(),
                                                  getReleaseVersion(),
                                                  getPassID()));
  }
} // namespace


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

  detail::ModuleFactory fact;
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

  art::MasterProductRegistry preg;
  art::WorkerParams params1(p1, p1, preg, table, "PROD");
  art::WorkerParams params2(p2, p2, preg, table, "PROD");

  std::unique_ptr<Worker> w1 = fact.makeWorker(params1, createModuleDescription(params1));
  std::unique_ptr<Worker> w2 = fact.makeWorker(params2, createModuleDescription(params2));

//  return 0;
}
