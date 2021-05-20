#define BOOST_TEST_MODULE (parse_path_spec_t)
#include "boost/test/unit_test.hpp"

#include "art/Utilities/detail/remove_whitespace.h"

#include <string>

BOOST_AUTO_TEST_SUITE(parse_path_spec_t)

BOOST_AUTO_TEST_CASE(remove_whitespace_t)
{
  std::string a("noblanks");
  std::string b("\t   no   blanks    \t");

  art::detail::remove_whitespace(b);
  BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_SUITE_END()
