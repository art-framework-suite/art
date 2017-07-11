#include "art/Framework/Core/detail/parse_path_spec.h"

#include <cassert>
#include <cstddef>
#include <string>
#include <vector>

void test_remove_whitespace() {
  std::string a("noblanks");
  std::string b("\t   no   blanks    \t");

  art::detail::remove_whitespace(b);
  assert(a == b);
}

void test_parse_path_spec() {
  std::vector<std::string> paths;
  paths.push_back("a:p1");
  paths.push_back("b:p2");
  paths.push_back("  c");
  paths.push_back("ddd\t:p3");
  paths.push_back("eee:  p4  ");

  std::vector<std::pair<std::string, std::string>> parsed(paths.size());
  for (size_t i = 0; i < paths.size(); ++i)
    art::detail::parse_path_spec(paths[i], parsed[i]);

  assert(parsed[0].first  == "a");
  assert(parsed[0].second == "p1");
  assert(parsed[1].first  == "b");
  assert(parsed[1].second == "p2");
  assert(parsed[2].first  == "");
  assert(parsed[2].second == "c");
  assert(parsed[3].first  == "ddd");
  assert(parsed[3].second == "p3");
  assert(parsed[4].first  == "eee");
  assert(parsed[4].second == "p4");
}

int main()
{
  test_remove_whitespace();
  test_parse_path_spec();
}
