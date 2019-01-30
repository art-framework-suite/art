#ifndef art_Framework_Art_detail_get_LibraryInfoCollection_h
#define art_Framework_Art_detail_get_LibraryInfoCollection_h

#include "art/Framework/Art/detail/LibraryInfoCollection.h"
#include "art/Utilities/PluginSuffixes.h"

#include <string>
#include <vector>

namespace cet {
  class LibraryManager;
}

namespace art::detail {

  constexpr char const*
  dflt_spec_pattern()
  {
    return "[A-Za-z0-9]+";
  }

  LibraryInfoCollection get_LibraryInfoCollection(std::string const& suffix,
                                                  std::string const& pattern,
                                                  bool verbose = false);
}

#endif /* art_Framework_Art_detail_get_LibraryInfoCollection_h */

// Local variables:
// mode: c++
// End:
