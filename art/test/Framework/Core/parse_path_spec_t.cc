#define BOOST_TEST_MODULE (parse_path_spec_t)
#include "boost/test/unit_test.hpp"

#include "art/Framework/Core/detail/parse_path_spec.h"
#include "art/Framework/Core/detail/remove_whitespace.h"

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

BOOST_AUTO_TEST_SUITE(parse_path_spec_t)

BOOST_AUTO_TEST_CASE(remove_whitespace_t)
{
  std::string a("noblanks");
  std::string b("\t   no   blanks    \t");

  art::detail::remove_whitespace(b);
  BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(parse_path_spec_t)
{
  std::vector<std::string> paths;
  paths.push_back("a:p1");
  paths.push_back("b:p2");
  paths.push_back("  c");
  paths.push_back("ddd\t:p3");
  paths.push_back("eee:  p4  ");

  std::vector<std::pair<std::string, std::string>> parsed(paths.size());
  for (size_t i = 0; i < paths.size(); ++i)
    parsed[i] = art::detail::parse_path_spec(paths[i]);

  BOOST_TEST(parsed[0].first == "a");
  BOOST_TEST(parsed[0].second == "p1");
  BOOST_TEST(parsed[1].first == "b");
  BOOST_TEST(parsed[1].second == "p2");
  BOOST_TEST(parsed[2].first == "");
  BOOST_TEST(parsed[2].second == "c");
  BOOST_TEST(parsed[3].first == "ddd");
  BOOST_TEST(parsed[3].second == "p3");
  BOOST_TEST(parsed[4].first == "eee");
  BOOST_TEST(parsed[4].second == "p4");
}

BOOST_AUTO_TEST_SUITE_END()
