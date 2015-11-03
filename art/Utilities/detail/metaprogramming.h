#ifndef art_Utilities_detail_metaprogramming_h
#define art_Utilities_detail_metaprogramming_h
// Common metaprogramming utilities.

namespace art {
  namespace detail {
    typedef char (& no_tag )[1]; // type indicating FALSE
    typedef char (& yes_tag)[2]; // type indicating TRUE

    // SFINAE magic for determining if type exists
    template <class T, class R = void>
    struct enable_if_type { using type = R; };
  }
}
#endif /* art_Utilities_detail_metaprogramming_h */

// Local Variables:
// mode: c++
// End:
