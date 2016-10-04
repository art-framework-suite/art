#ifndef art_Utilities_BasicHelperMacros_h
#define art_Utilities_BasicHelperMacros_h

// =====================================================================
//
// HelperMacros
//
// =====================================================================

#include "boost/filesystem.hpp"
#include "cetlib/detail/metaprogramming.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Table.h"

#include <ostream>
#include <string>

namespace bfs = boost::filesystem;

// =====================================================================

#define PROVIDE_FILE_PATH()                              \
  extern "C"                                             \
  std::string get_source_location()                      \
  {                                                      \
    bfs::path const p {__FILE__};                        \
    return bfs::complete(p).native();                    \
  }

// =====================================================================

namespace art {
  namespace detail {

    template<class T, class Enable = void>
    struct MaybePrintDescription{
      std::ostream& operator()(std::ostream& os, std::string const& /*name*/, std::string const& prefix)
      {
        return os << "\n" << prefix << "[ None provided ]\n";
      }
    };

    template<class T>
    struct MaybePrintDescription<T, cet::detail::enable_if_type_exists_t<typename T::Parameters>>
    {
      std::ostream& operator()(std::ostream& os, std::string const& name, std::string const& prefix)
      {
        typename T::Parameters{fhicl::Name(name)}.print_allowed_configuration(os, prefix);
        return os;
      }
    };

    struct ToolConfig {
      fhicl::Atom<std::string> tool_type { fhicl::Name("tool_type") };
    };

  }
}

#define PROVIDE_DESCRIPTION(klass)                                      \
  extern "C"                                                            \
  std::ostream& print_description(std::ostream& os, std::string const& name, std::string const& prefix) \
  {                                                                     \
    art::detail::MaybePrintDescription<klass> print;                    \
    return print(os, name, prefix);                                     \
  }

#define PROVIDE_TOOL_FUNCTION_DESCRIPTION()                             \
  extern "C"                                                            \
  std::ostream& print_description(std::ostream& os, std::string const& name, std::string const& prefix) \
  {                                                                     \
    fhicl::Table<art::detail::ToolConfig> tc { fhicl::Name(name) };     \
    tc.print_allowed_configuration(os, prefix);                         \
    return os;                                                          \
  }

#endif /* art_Utilities_BasicHelperMacros_h */

// Local Variables:
// mode: c++
// End:
