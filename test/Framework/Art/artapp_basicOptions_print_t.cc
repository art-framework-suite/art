#include "art/Framework/Art/artapp.h"

#define BOOST_TEST_MODULE ( artapp printTests )
#include "boost/test/auto_unit_test.hpp"

#include "cetlib/exception.h"

BOOST_AUTO_TEST_SUITE(artappBasicOptionsPrintTests)

BOOST_AUTO_TEST_CASE(PrintAvailableModules)
{
  char const * strings[] = { "artapp_t","--print-available-modules" };
  BOOST_REQUIRE(artapp(2, const_cast<char **>(strings)) == 1);
}

BOOST_AUTO_TEST_CASE(PrintAvailableServices)
{
  char const* strings[] = { "artapp_t","--print-available-services" };
  BOOST_REQUIRE(artapp(2, const_cast<char **>(strings)) == 1 );
}

BOOST_AUTO_TEST_SUITE_END()
