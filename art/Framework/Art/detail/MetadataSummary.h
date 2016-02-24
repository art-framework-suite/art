#ifndef art_Framework_Art_detail_MetadataSummary_h
#define art_Framework_Art_detail_MetadataSummary_h

#include "art/Utilities/PluginSuffixes.h"

#include <string>
#include <vector>

namespace art {
  namespace detail {
    class LibraryInfo;

    template <art::suffix_type S>
    class MetadataSummaryFor;
  }
}

namespace art {
  namespace detail {

    class MetadataSummary {
    public:

      std::string header() const { return doHeader(); }
      std::string summary(LibraryInfo const& li) const { return doSummary(li); }
      std::vector<std::size_t> const& widths() const { return doWidths(); }
      virtual ~MetadataSummary() = default;

    private:

      virtual std::string doHeader() const = 0;
      virtual std::string doSummary(LibraryInfo const& li) const = 0;
      virtual std::vector<std::size_t> const& doWidths() const = 0;

    };

  }
}

#endif /* art_Framework_Art_detail_MetadataSummary_h */

// Local variables:
// mode: c++
// End:
