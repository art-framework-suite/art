#ifndef art_Utilities_BasicHelperMacros_h
#define art_Utilities_BasicHelperMacros_h

////////////////////////////////////////////////////////////////////////
//
// HelperMacros
//
////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <string>
#include "art/Utilities/detail/metaprogramming.h"
#include "boost/filesystem.hpp"

namespace bfs = boost::filesystem;

// =====================================================================

#define PROVIDE_FILE_PATH()                              \
  extern "C"                                             \
  std::string                                            \
  get_source_location()                                  \
  {                                                      \
    bfs::path const p( __FILE__ );                       \
    return bfs::complete(p).native();                    \
  }

// =====================================================================

namespace art {
  namespace detail {

    template<class T, class Enable = void>
    struct MaybePrintDescription{
      std::ostream & operator()(std::ostream & os, std::string const& prefix)
      {
        return os << "\n" << prefix << "[ None provided ]\n";
      }
    };

    template<class T>
    struct MaybePrintDescription<T, typename enable_if_type<typename T::Parameters>::type>
    {
      std::ostream & operator()(std::ostream & os, std::string const& prefix)
      {
        typename T::Parameters().print_allowed_configuration(os, prefix);
        return os;
      }
    };

  }
}

#define PROVIDE_DESCRIPTION( klass )                                    \
  extern "C"                                                            \
  std::ostream&                                                         \
  print_description(std::ostream& os, std::string const& prefix)        \
  {                                                                     \
    art::detail::MaybePrintDescription<klass> print;                    \
    return print(os,prefix);                                            \
  }                                                                     \


#endif /* art_Utilities_BasicHelperMacros_h */

// Local Variables:
// mode: c++
// End:
