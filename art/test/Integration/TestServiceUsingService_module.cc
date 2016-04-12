////////////////////////////////////////////////////////////////////////
// Class:       TestServiceUsingService
// Module Type: analyzer
// File:        TestServiceUsingService_module.cc
//
// Generated at Thu Feb 23 09:55:50 2012 by Chris Green using artmod
// from art v1_00_08.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "cetlib/quiet_unit_test.hpp"
#include "art/test/Integration/Reconfigurable.h"
#include "art/test/Integration/ServiceUsing.h"
#include "art/test/Integration/Wanted.h"

namespace arttest {
  class TestServiceUsingService;
}

class arttest::TestServiceUsingService : public art::EDAnalyzer {
public:
  explicit TestServiceUsingService(fhicl::ParameterSet const &);
  ~TestServiceUsingService();

  void analyze(art::Event const &) override;

  void beginJob() override;
  void endJob() override;

private:

  int debug_level_;

};


arttest::TestServiceUsingService::TestServiceUsingService(fhicl::ParameterSet const &p)
 :
  art::EDAnalyzer(p),
  debug_level_(art::ServiceHandle<ServiceUsing>()->getCachedValue())
{
}

arttest::TestServiceUsingService::~TestServiceUsingService()
{
  // Test that art::ServiceHandle can be dereferenced in a module destructor
  art::ServiceHandle<ServiceUsing>()->getCachedValue();
}


void arttest::TestServiceUsingService::analyze(art::Event const &)
{
  // NOP.
}

void arttest::TestServiceUsingService::beginJob()
{
  BOOST_CHECK_EQUAL(debug_level_,
                    art::ServiceHandle<ServiceUsing>()->getCachedValue());
  BOOST_CHECK_EQUAL(art::ServiceHandle<Reconfigurable>()->get_debug_level(),
                    art::ServiceHandle<ServiceUsing>()->getCachedValue());
}

void arttest::TestServiceUsingService::endJob()
{
  art::ServiceHandle<Reconfigurable> reconfigurable;
  BOOST_CHECK(reconfigurable->postBeginJobCalled());
  BOOST_CHECK(art::ServiceHandle<Wanted>()->postBeginJobCalled());

  int const current_value = art::ServiceHandle<ServiceUsing>()->getCachedValue();
  BOOST_CHECK_NE(debug_level_, current_value);
  BOOST_CHECK_EQUAL(reconfigurable->get_debug_level(), current_value);
}

DEFINE_ART_MODULE(arttest::TestServiceUsingService)
