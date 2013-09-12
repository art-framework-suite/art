#define BOOST_TEST_MODULE(ServiceDirector_t)
#include "boost/test/auto_unit_test.hpp"

#include "art/Framework/EventProcessor/ServiceDirector.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/make_ParameterSet.h"

namespace {
  bool verifyException(art::Exception const & e,
                       art::errors::ErrorCodes category,
                       std::string const &whatString)
  {
    auto const & cmp = e.what();
    return e.categoryCode() == category &&
      cmp == whatString;
  }

  struct TestFixture {
    art::ActivityRegistry areg;
    art::ServiceToken token;
  };
}


BOOST_FIXTURE_TEST_SUITE(ServiceDirector_t, TestFixture)

BOOST_AUTO_TEST_CASE(Construct)
{
  std::vector<std::tuple<std::string, art::errors::ErrorCodes, std::string>> test_sets;
  test_sets.emplace_back
    ("process_name: \"test\" "
     "services.scheduler.num_schedules: 2",
     static_cast<art::errors::ErrorCodes>(0),
     "");
  for (auto const & test : test_sets) {
    fhicl::ParameterSet ps;
    fhicl::make_ParameterSet(std::get<0>(test), ps);
    try {
      art::ServiceDirector sd(ps, areg, token);
    }
    catch (art::Exception const & e) {
      if (!verifyException(e, std::get<1>(test), std::get<2>(test))) {
        throw;
      }
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
