#ifndef art_Framework_Core_detail_parse_path_spec_h
#define art_Framework_Core_detail_parse_path_spec_h

#include <string>
#include <utility>

namespace art::detail {
  void remove_whitespace(std::string&);
  void parse_path_spec(std::string path_spec,
                       std::pair<std::string, std::string>& output);
} // namespace art::detail

#endif /* art_Framework_Core_detail_parse_path_spec_h */

// Local Variables:
// mode: c++
// End:
