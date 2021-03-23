#ifndef art_Framework_Core_detail_parse_path_spec_h
#define art_Framework_Core_detail_parse_path_spec_h

#include <string>
#include <utility>

namespace art::detail {
  // Process name <-> Path name
  std::pair<std::string, std::string> parse_path_spec(std::string path_spec);
}

#endif /* art_Framework_Core_detail_parse_path_spec_h */

// Local Variables:
// mode: c++
// End:
