#ifndef art_Framework_Art_detail_MetadataRegexHelpers_h
#define art_Framework_Art_detail_MetadataRegexHelpers_h

#include <regex>
#include <string>

namespace art {
  namespace detail {

    std::regex regex_for_spec(std::string const&);
    void replace_type(std::string& str, std::string const& spec, std::regex const& r);

  }
}

#endif /* art_Framework_Art_detail_MetadataRegexHelpers_h */

// Local variables:
// mode: c++
// End:
