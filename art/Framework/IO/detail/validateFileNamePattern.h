#ifndef art_Framework_IO_detail_validateFileNamePattern_h
#define art_Framework_IO_detail_validateFileNamePattern_h

#include <string>

namespace art::detail {
  void validateFileNamePattern(bool do_check, std::string const& pattern);
}

#endif /* art_Framework_IO_detail_validateFileNamePattern_h */

// Local Variables:
// mode: c++
// End:
