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


// C++ support:
#include <algorithm>  // std::transform
#include <iterator>   // std::back_inserter
#include <vector>     // std::vector


// ======================================================================


namespace edm {

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

}  // namespace edm


// ======================================================================


#endif  // FWCore_Utilities_vectorTransform_hh
