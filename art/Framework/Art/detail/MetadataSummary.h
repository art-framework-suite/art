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
  } // namespace detail
} // namespace art

namespace art {
  namespace detail {

    struct Summary {
      std::string message;
      bool is_duplicate;
    };

    class MetadataSummary {
    public:
      std::string
      header() const
      {
        return doHeader();
      }
      Summary
      summary(LibraryInfo const& li, std::size_t const entry) const
      {
        return doSummary(li, entry);
      }
      std::vector<std::size_t> const&
      widths() const
      {
        return doWidths();
      }
      virtual ~MetadataSummary() = default;

    private:
      virtual std::string doHeader() const = 0;
      virtual Summary doSummary(LibraryInfo const& li,
                                std::size_t entry) const = 0;
      virtual std::vector<std::size_t> const& doWidths() const = 0;
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Art_detail_MetadataSummary_h */

// Local variables:
// mode: c++
// End:
