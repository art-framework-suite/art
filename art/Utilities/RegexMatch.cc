// functions used to assist with regular expression matching of strings

#include "art/Utilities/RegexMatch.h"
#include "boost/algorithm/string.hpp"
#include <string>
#include <vector>

namespace art {

  bool is_glob(std::string const& pattern) {
    return (pattern.find_first_of("*?") != std::string::npos);
  }

  std::string glob2reg(std::string pattern) {
    boost::replace_all(pattern, "*", ".*");
    boost::replace_all(pattern, "?", ".");
    return pattern;
  }

  std::vector<std::vector<std::string>::const_iterator>
  regexMatch(std::vector<std::string> const& strings, std::regex const& regexp) {
    std::vector< std::vector<std::string>::const_iterator> matches;
    for (auto i = strings.begin(), iEnd = strings.end(); i != iEnd; ++i) {
      if (std::regex_match((*i), regexp)) {
        matches.push_back(i);
      }
    }
    return matches;
  }

  std::vector<std::vector<std::string>::const_iterator>
  regexMatch(std::vector<std::string> const& strings, std::string const& pattern) {
    std::regex const regexp{glob2reg(pattern)};
    return regexMatch(strings, regexp);
  }

}
