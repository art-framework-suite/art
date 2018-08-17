#ifndef art_Framework_Art_detail_get_MetadataSummary_h
#define art_Framework_Art_detail_get_MetadataSummary_h

#include "art/Framework/Art/detail/LibraryInfoCollection.h"
#include "art/Framework/Art/detail/MetadataSummary.h"
#include "art/Utilities/PluginSuffixes.h"

#include <memory>

namespace art::detail {

  std::unique_ptr<MetadataSummary> get_MetadataSummary(
    std::string const& suffix,
    LibraryInfoCollection const& coll);
}

#endif /* art_Framework_Art_detail_get_MetadataSummary_h */

// Local variables:
// mode: c++
// End:
