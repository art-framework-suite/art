#ifndef art_Utilities_detail_remove_whitespace_h
#define art_Utilities_detail_remove_whitespace_h

#include <string>

namespace art::detail {
  void remove_whitespace(std::string& str);
  bool has_whitespace(std::string const& str);
}

#endif /* art_Utilities_detail_remove_whitespace_h */

// Local Variables:
// mode: c++
// End:
