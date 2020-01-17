#define BOOST_TEST_MODULE (fhicl_key test)
#include "art/Framework/Art/detail/fhicl_key.h"
#include "cetlib/quiet_unit_test.hpp"

#include <string>

using namespace std::string_literals;
using art::detail::fhicl_key;

BOOST_AUTO_TEST_SUITE(fhicl_key_t)

BOOST_AUTO_TEST_CASE(one_arg)
{
  BOOST_TEST(fhicl_key("a") == "a"s);
  BOOST_TEST(fhicl_key("b"s) == "b"s);
}

BOOST_AUTO_TEST_CASE(multiple_args)
{
  BOOST_TEST(fhicl_key("a", "b") == "a.b"s);
  BOOST_TEST(fhicl_key("a", "b", "c") == "a.b.c"s);
  auto const& stem = fhicl_key("a", "b");
  BOOST_TEST(fhicl_key(stem, "c") == "a.b.c"s);
  BOOST_TEST(fhicl_key("", "a") == "a"s);
}

BOOST_AUTO_TEST_SUITE_END()
