#ifndef art_Framework_Art_detail_MetadataCollector_h
#define art_Framework_Art_detail_MetadataCollector_h

#include "art/Framework/Art/detail/PluginMetadata.h"
#include "art/Utilities/PluginSuffixes.h"

#include <string>

namespace art::detail {
  class LibraryInfo;

  template <art::suffix_type S>
  class MetadataCollectorFor;

  class MetadataCollector {
  public:
    PluginMetadata
    collect(LibraryInfo const& li,
            std::string const& prefix,
            std::string const& header_label,
            std::string const& param_to_replace) const
    {
      return doCollect(li, prefix, header_label, param_to_replace);
    }
    virtual ~MetadataCollector() = default;

  private:
    virtual PluginMetadata doCollect(
      LibraryInfo const& li,
      std::string const& prefix,
      std::string const& header_label,
      std::string const& param_to_replace) const = 0;
  };

} // namespace art::detail

#endif /* art_Framework_Art_detail_MetadataCollector_h */

// Local variables:
// mode: c++
// End:
