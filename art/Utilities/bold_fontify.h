#ifndef art_Utilities_bold_fontify_h
#define art_Utilities_bold_fontify_h

#include <string>

namespace art {
  namespace detail {
    inline std::string
    bold_fontify(std::string const& s)
    {
      return "\033[1m" + s + "\033[0m";
    }
  } // namespace detail
} // namespace art

#endif /* art_Utilities_bold_fontify_h */

// Local variables:
// Mode: c++
// End:
