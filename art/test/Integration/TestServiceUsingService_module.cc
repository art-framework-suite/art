#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/test/Integration/ServiceUsing.h"
#include "art/test/Integration/Wanted.h"
#include "cetlib/quiet_unit_test.hpp"

namespace art {
namespace test {

class TestServiceUsingService : public EDAnalyzer {

public:

  ~TestServiceUsingService();

  explicit
  TestServiceUsingService(fhicl::ParameterSet const&);

  void
  analyze(art::Event const &) override;

  void
  beginJob() override;

  void
  endJob() override;

private:

  int debug_level_;

};

TestServiceUsingService::
~TestServiceUsingService()
{
  // Test that art::ServiceHandle can be dereferenced in a module destructor
  ServiceHandle<ServiceUsing const>{}->getCachedValue();
}

TestServiceUsingService::
TestServiceUsingService(fhicl::ParameterSet const& pset)
  : EDAnalyzer{pset}
  , debug_level_{ServiceHandle<ServiceUsing const>()->getCachedValue()}
{
}

void
TestServiceUsingService::
analyze(Event const&)
{
}

void
TestServiceUsingService::
beginJob()
{
  ServiceHandle<ServiceUsing const> sus;
  BOOST_CHECK_EQUAL(debug_level_, sus->getCachedValue());
  BOOST_CHECK_EQUAL(ServiceHandle<Wanted const>{}->getCachedValue(), sus->getCachedValue());
}

void
TestServiceUsingService::
endJob()
{
  ServiceHandle<ServiceUsing const> sus;
  BOOST_CHECK(sus->postBeginJobCalled());
  int const current_value = sus->getCachedValue();
  BOOST_CHECK_NE(debug_level_, current_value);
  BOOST_CHECK_EQUAL(ServiceHandle<Wanted const>{}->getCachedValue(), current_value);
}

} // namespace test
} // namespace art

DEFINE_ART_MODULE(art::test::TestServiceUsingService)
