// functions shared between the art::EventSelector and the HLTHighLevel filter

#ifndef art_Utilities_RegexMatch_h
#define art_Utilities_RegexMatch_h

#include <regex>
#include <string>
#include <vector>

// ----------------------------------------------------------------------

namespace art {

  bool is_glob(std::string const& pattern);

  std::string glob2reg(std::string pattern);

  std::vector<std::vector<std::string>::const_iterator> regexMatch(
    std::vector<std::string> const& strings,
    std::regex const& regexp);

  std::vector<std::vector<std::string>::const_iterator> regexMatch(
    std::vector<std::string> const& strings,
    std::string const& pattern);

} // art

// ======================================================================

#endif /* art_Utilities_RegexMatch_h */

// Local Variables:
// mode: c++
// End:
