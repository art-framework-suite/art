#ifndef art_Framework_Art_detail_get_LibraryInfoCollection_h
#define art_Framework_Art_detail_get_LibraryInfoCollection_h

#include "art/Framework/Art/detail/LibraryInfoCollection.h"
#include "art/Utilities/PluginSuffixes.h"

#include <string>
#include <vector>

namespace cet {
  class LibraryManager;
}

namespace art {
  namespace detail {

    constexpr const char*
    dflt_spec_pattern()
    {
      return "[A-Za-z0-9]+";
    }

    LibraryInfoCollection get_LibraryInfoCollection(suffix_type suffix,
                                                    std::string const& pattern,
                                                    bool const verbose = false);

  } // namespace detail
} // namespace art

#endif /* art_Framework_Art_detail_get_LibraryInfoCollection_h */

// Local variables:
// mode: c++
// End:
