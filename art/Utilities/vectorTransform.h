#ifndef FWCore_Utilities_vectorTransform_hh
#define FWCore_Utilities_vectorTransform_hh

// ======================================================================
//
// Copy a vector of element type From to a vector of element type To.
//
// This assumes the following works:
//   From f;
//   To   t = static_cast<To>(f);
//
// ======================================================================

#include <algorithm>
#include <iterator>
#include <vector>

// ======================================================================

namespace art {

  namespace {
    template< class From, class To >
      inline To
      cast_one( From value )
    {
      return static_cast<To>( value );
    }
  }  // namespace

  template< class From, class To >
    inline void
    vectorTransform( std::vector<From> const & in
                   , std::vector<To>         & out
                   )
  {
    out.clear();
    out.reserve( in.size() );
    std::transform( in.begin(), in.end()
                  , std::back_inserter(out)
                  , cast_one<From,To>
                  );
  }  // vectorTransform<,>()

}  // art

// ======================================================================

#endif
