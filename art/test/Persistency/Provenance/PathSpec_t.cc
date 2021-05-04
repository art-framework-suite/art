#define BOOST_TEST_MODULE (PathSpec_t)
#include "boost/test/unit_test.hpp"

#include "art/Persistency/Provenance/PathSpec.h"
#include "art/Utilities/detail/remove_whitespace.h"

#include <cstddef>
#include <string>
#include <vector>

BOOST_AUTO_TEST_SUITE(PathSpec_t)

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
    parsed[i] = art::split_process_and_path_names(paths[i]);

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
