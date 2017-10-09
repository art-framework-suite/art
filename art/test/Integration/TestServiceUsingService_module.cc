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
#include "art/test/Integration/ServiceUsing.h"
#include "art/test/Integration/Wanted.h"
#include "cetlib/quiet_unit_test.hpp"

namespace art {
  namespace test {
    class TestServiceUsingService;
  }
}

class art::test::TestServiceUsingService : public EDAnalyzer {
public:
  explicit TestServiceUsingService(fhicl::ParameterSet const&);
  ~TestServiceUsingService();

  void analyze(art::Event const&) override;

  void beginJob() override;
  void endJob() override;

private:
  int debug_level_;
};

art::test::TestServiceUsingService::TestServiceUsingService(
  fhicl::ParameterSet const& p)
  : EDAnalyzer{p}
  , debug_level_{ServiceHandle<ServiceUsing const>()->getCachedValue()}
{}

art::test::TestServiceUsingService::~TestServiceUsingService()
{
  // Test that art::ServiceHandle can be dereferenced in a module destructor
  ServiceHandle<ServiceUsing const> {}
  ->getCachedValue();
}

void
art::test::TestServiceUsingService::analyze(Event const&)
{
  // NOP.
}

void
art::test::TestServiceUsingService::beginJob()
{
  ServiceHandle<ServiceUsing const> sus;
  BOOST_CHECK_EQUAL(debug_level_, sus->getCachedValue());
  BOOST_CHECK_EQUAL(ServiceHandle<Wanted const> {}->getCachedValue(),
                    sus->getCachedValue());
}

void
art::test::TestServiceUsingService::endJob()
{
  ServiceHandle<ServiceUsing const> sus;
  BOOST_CHECK(sus->postBeginJobCalled());

  int const current_value{sus->getCachedValue()};
  BOOST_CHECK_NE(debug_level_, current_value);
  BOOST_CHECK_EQUAL(ServiceHandle<Wanted const> {}->getCachedValue(),
                    current_value);
}

DEFINE_ART_MODULE(art::test::TestServiceUsingService)
