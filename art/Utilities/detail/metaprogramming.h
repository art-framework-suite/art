#ifndef art_Utilities_detail_metaprogramming_h
#define art_Utilities_detail_metaprogramming_h
// Common metaprogramming utilities.

namespace art {
  namespace detail {
    typedef char (& no_tag )[1]; // type indicating FALSE
    typedef char (& yes_tag)[2]; // type indicating TRUE
  }
}
#endif /* art_Utilities_detail_metaprogramming_h */

// Local Variables:
// mode: c++
// End:
