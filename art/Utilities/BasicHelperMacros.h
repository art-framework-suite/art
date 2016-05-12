#ifndef art_Utilities_BasicHelperMacros_h
#define art_Utilities_BasicHelperMacros_h

////////////////////////////////////////////////////////////////////////
//
// HelperMacros
//
////////////////////////////////////////////////////////////////////////

#include <ostream>
#include <string>
#include "canvas/Utilities/detail/metaprogramming.h"
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
    struct MaybePrintDescription<T, enable_if_type_exists_t<typename T::Parameters>>
    {
      std::ostream & operator()(std::ostream & os, std::string const& prefix)
      {
        typename T::Parameters{}.print_allowed_configuration(os, prefix);
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
