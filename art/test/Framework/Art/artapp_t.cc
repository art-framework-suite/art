#include "art/Framework/Art/artapp.h"

#define BOOST_TEST_MODULE (artapp test)
#include "cetlib/quiet_unit_test.hpp"

#include "cetlib/exception.h"

BOOST_AUTO_TEST_SUITE(artappTests)

BOOST_AUTO_TEST_CASE(NoConfig)
{
  char const* strings[] = {"artapp_t"};
  BOOST_REQUIRE(artapp(1, const_cast<char**>(strings)) == 89);
}

BOOST_AUTO_TEST_CASE(testHelp)
{
  char const* strings[] = {"artapp_t", "--help"};
  BOOST_REQUIRE(artapp(2, const_cast<char**>(strings)) == 1);
}

BOOST_AUTO_TEST_CASE(testBadConfigOption)
{
  char const* strings[] = {"artapp_t", "--config"};
  BOOST_REQUIRE(artapp(2, const_cast<char**>(strings)) == 88);
}

BOOST_AUTO_TEST_CASE(testEmptyConfig)
{
  char const* strings[] = {"artapp_t", "--config", "opt-empty.fcl"};
  BOOST_REQUIRE(artapp(3, const_cast<char**>(strings)) == 0);
}

BOOST_AUTO_TEST_CASE(testNonesuchConfig)
{
  char const* strings[] = {"artapp_t", "--config", "no_such_config.fcl"};
  BOOST_REQUIRE(artapp(3, const_cast<char**>(strings)) == 90);
}

BOOST_AUTO_TEST_SUITE_END()
