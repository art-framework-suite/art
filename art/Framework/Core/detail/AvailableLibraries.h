#ifndef art_Framework_Core_detail_AvailableLibraries_h
#define art_Framework_Core_detail_AvailableLibraries_h

#include "art/Framework/Core/ModuleType.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <map>

namespace art {
  namespace detail {

    struct LibraryInfo {

      LibraryInfo(std::string const& name,
                  std::string const& so,
                  std::string const& src,
                  std::string const& type,
                  std::string const& d)
        : altname(name),library(so),source(src),type(type),description(d) {}

      std::string altname;
      std::string library;
      std::string source;
      std::string type;
      std::string description;

    };

    using LibraryInfoContainer = std::multimap<std::string,LibraryInfo>;

    constexpr const char* dflt_pattern() {
      return "([-A-Za-z0-9]*_)*[A-Za-z0-9]+_";
    }

    void
    getModuleLibraries( LibraryInfoContainer& map,
                        std::string const & pattern = dflt_pattern(),
                        std::string const & prefix  = "");

    void
    getSourceLibraries( LibraryInfoContainer& map,
                        std::string const & pattern = dflt_pattern());

    void
    getServiceLibraries( LibraryInfoContainer& map,
                         std::string const & pattern = dflt_pattern(),
                         std::string const & prefix  = "");

  } // detail
} // art

#endif //art_Framework_Core_detail_AvailableLibraries_h

// Local variables:
// mode: c++
// End:
