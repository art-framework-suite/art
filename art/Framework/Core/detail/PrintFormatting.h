#ifndef art_Framework_Core_detail_PrintFormatting_h
#define art_Framework_Core_detail_PrintFormatting_h

#include <string>

namespace art {
  namespace detail {

    //=========================================================================
    // bold-fontifier
    struct font_bold {
      font_bold(std::string const& str ) : instance(str){}
      std::string instance;
    };

    inline std::ostream& operator<< ( std::ostream& os, font_bold const && fb ) {
      return os <<  "\033[1m" << fb.instance << "\033[0m";
    }

    inline std::string indent0()   { return std::string(3, ' '); }
    inline std::string indent_1()  { return std::string(4, ' '); }
    inline std::string indent__2() { return std::string(8, ' '); }
  }
}

#endif /* art_Framework_Core_detail_PrintFormatting_h */

// Local variables:
// mode: c++
// End:
