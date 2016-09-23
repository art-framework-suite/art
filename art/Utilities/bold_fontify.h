#ifndef art_Framework_Art_detail_bold_fontify_h
#define art_Framework_Art_detail_bold_fontify_h

#include <string>

namespace art {
  namespace detail {
    inline std::string bold_fontify(std::string const& s)
    {
      return "\033[1m"+s+"\033[0m";
    }
  }
}

#endif

// Local variables:
// Mode: c++
// End:
