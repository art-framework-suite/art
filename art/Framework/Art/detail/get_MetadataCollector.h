#ifndef art_Framework_Art_detail_getMetadataCollector_h
#define art_Framework_Art_detail_getMetadataCollector_h

#include "art/Framework/Art/detail/MetadataCollector.h"
#include "art/Utilities/PluginSuffixes.h"

#include <memory>

namespace art {
  namespace detail {
    std::unique_ptr<MetadataCollector> get_MetadataCollector(suffix_type st);
  }
}

#endif

// Local variables:
// mode: c++
// End:
