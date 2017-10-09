#ifndef art_Framework_Art_detail_MetadataCollector_h
#define art_Framework_Art_detail_MetadataCollector_h

#include "art/Framework/Art/detail/PluginMetadata.h"
#include "art/Utilities/PluginSuffixes.h"

#include <string>

namespace art {
  namespace detail {
    class LibraryInfo;

    template <art::suffix_type S>
    class MetadataCollectorFor;
  } // namespace detail
} // namespace art

namespace art {
  namespace detail {

    class MetadataCollector {
    public:
      PluginMetadata
      collect(LibraryInfo const& li, std::string const& prefix) const
      {
        return doCollect(li, prefix);
      }
      virtual ~MetadataCollector() = default;

    private:
      virtual PluginMetadata doCollect(LibraryInfo const& li,
                                       std::string const& prefix) const = 0;
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Art_detail_MetadataCollector_h */

// Local variables:
// mode: c++
// End:
