#define BOOST_TEST_MODULE (artapp test)
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Art/artapp.h"
#include "art/Framework/Art/detail/info_success.h"
#include "cetlib_except/exception.h"

BOOST_AUTO_TEST_SUITE(artappTests)

BOOST_AUTO_TEST_CASE(NoConfig)
{
  char const* strings[] = {"artapp_t"};
  BOOST_REQUIRE(artapp(1, const_cast<char**>(strings)) == 89);
}

BOOST_AUTO_TEST_CASE(testHelp)
{
  char const* strings[] = {"artapp_t", "--help"};
  BOOST_REQUIRE(artapp(2, const_cast<char**>(strings)) ==
                art::detail::info_success());
}

BOOST_AUTO_TEST_CASE(testBadConfigOption)
{
  char const* strings[] = {"artapp_t", "--config"};
  BOOST_REQUIRE(artapp(2, const_cast<char**>(strings)) == 88);
}

BOOST_AUTO_TEST_CASE(testEmptyConfig)
{
  char const* strings[] = {"artapp_t", "--config", "/dev/null"};
  BOOST_REQUIRE(artapp(3, const_cast<char**>(strings)) == 0);
}

BOOST_AUTO_TEST_CASE(testNonesuchConfig)
{
  char const* strings[] = {"artapp_t", "--config", "no_such_config.fcl"};
  BOOST_REQUIRE(artapp(3, const_cast<char**>(strings)) == 90);
}

// Processing options
BOOST_AUTO_TEST_CASE(testParallelism1)
{
  char const* strings[] = {"artapp_t", "--config", "/dev/null", "-j4"};
  BOOST_REQUIRE(artapp(4, const_cast<char**>(strings)) == 0);
}

BOOST_AUTO_TEST_CASE(testParallelism2)
{
  char const* strings[] = {"artapp_t", "--config", "/dev/null", "--nthreads=1"};
  BOOST_REQUIRE(artapp(4, const_cast<char**>(strings)) == 0);
}

BOOST_AUTO_TEST_CASE(testParallelism3)
{
  char const* strings[] = {
    "artapp_t", "--config", "/dev/null", "--nschedules=1"};
  BOOST_REQUIRE(artapp(4, const_cast<char**>(strings)) == 0);
}

BOOST_AUTO_TEST_CASE(testParallelism4)
{
  char const* strings[] = {
    "artapp_t", "--config", "/dev/null", "--nschedules=1", "--nthreads=1"};
  BOOST_REQUIRE(artapp(5, const_cast<char**>(strings)) == 0);
}

BOOST_AUTO_TEST_CASE(testParallelism5)
{
  char const* strings[] = {
    "artapp_t", "--config", "/dev/null", "-j1", "--nschedules=1"};
  BOOST_REQUIRE(artapp(5, const_cast<char**>(strings)) == 89);
}

BOOST_AUTO_TEST_CASE(testParallelism6)
{
  char const* strings[] = {
    "artapp_t", "--config", "/dev/null", "-j1", "--nthreads=1"};
  BOOST_REQUIRE(artapp(5, const_cast<char**>(strings)) == 89);
}

BOOST_AUTO_TEST_SUITE_END()
