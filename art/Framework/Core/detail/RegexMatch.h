// functions shared between the art::EventSelector and the HLTHighLevel filter

#ifndef art_Framework_Core_detail_RegexMatch_h
#define art_Framework_Core_detail_RegexMatch_h

#include <string>
#include <vector>

// ----------------------------------------------------------------------

namespace art {

  bool is_glob(std::string const& pattern);

  std::string glob2reg(std::string pattern);

  std::vector<std::vector<std::string>::const_iterator> regexMatch(
    std::vector<std::string> const& strings,
    std::string const& pattern);

} // namespace art

// ======================================================================

#endif /* art_Framework_Core_detail_RegexMatch_h */

// Local Variables:
// mode: c++
// End:
