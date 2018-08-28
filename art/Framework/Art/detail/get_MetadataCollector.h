#ifndef art_Framework_Art_detail_get_MetadataCollector_h
#define art_Framework_Art_detail_get_MetadataCollector_h

#include "art/Framework/Art/detail/MetadataCollector.h"

#include <memory>

namespace art::detail {
  std::unique_ptr<MetadataCollector> get_MetadataCollector(
    std::string const& suffix);
}

#endif /* art_Framework_Art_detail_get_MetadataCollector_h */

// Local variables:
// mode: c++
// End:
