#include "art/Framework/Core/detail/RegexMatch.h"
#include "cetlib/replace_all.h"

#include <regex>
#include <string>
#include <vector>

namespace art {

  bool
  is_glob(std::string const& pattern)
  {
    return pattern.find_first_of("*?") != std::string::npos;
  }

  std::string
  glob2reg(std::string pattern)
  {
    cet::replace_all(pattern, "*", ".*");
    cet::replace_all(pattern, "?", ".");
    return pattern;
  }

  std::vector<std::vector<std::string>::const_iterator>
  regexMatch(std::vector<std::string> const& strings,
             std::string const& pattern)
  {
    auto const reg_str = glob2reg(pattern);
    // We allow for a trigger-bit to lead the trigger path name.
    std::regex const regexp{"(\\d+:)?" + glob2reg(pattern)};
    std::vector<std::vector<std::string>::const_iterator> result;
    for (auto it = strings.begin(), e = strings.end(); it != e; ++it) {
      if (std::regex_match(*it, regexp)) {
        result.push_back(it);
      }
    }
    return result;
  }

} // namespace art
