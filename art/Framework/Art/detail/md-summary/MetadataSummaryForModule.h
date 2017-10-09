#ifndef art_Framework_Art_detail_md_summary_MetadataSummaryForModule_h
#define art_Framework_Art_detail_md_summary_MetadataSummaryForModule_h

#include "art/Framework/Art/detail/LibraryInfoCollection.h"
#include "art/Framework/Art/detail/MetadataSummary.h"
#include "art/Framework/Art/detail/PrintFormatting.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace art {
  namespace detail {

    template <>
    class MetadataSummaryFor<suffix_type::module> : public MetadataSummary {
    public:
      MetadataSummaryFor(LibraryInfoCollection const& coll)
        : coll_{coll}
        , widths_{std::to_string(coll.size()).size(),
                  columnWidth(coll, &LibraryInfo::short_spec, "module_type"),
                  columnWidth(coll, &LibraryInfo::plugin_type, "Type"),
                  columnWidth(coll, &LibraryInfo::path, "Source location")}
      {}

    private:
      LibraryInfoCollection const& coll_;
      Widths widths_;

      Widths const&
      doWidths() const override
      {
        return widths_;
      }

      std::string
      doHeader() const override
      {
        std::ostringstream result;
        result << indent(widths_[0] + 2) << std::setw(widths_[1] + 4)
               << std::left << "module_type" << std::setw(widths_[2] + 4)
               << std::left << "Type" << std::left << "Source location";
        return result.str();
      }

      Summary
      doSummary(LibraryInfo const& li, std::size_t const entry) const override
      {
        auto const count = std::count_if(
          coll_.cbegin(), coll_.cend(), LibraryInfoMatch{li.short_spec()});
        bool const is_duplicate = count != 1;
        auto const dupl = is_duplicate ? '*' : ' ';

        std::ostringstream result;
        result << std::setw(widths_[0]) << std::right << entry << '.' << dupl
               << std::setw(widths_[1] + 4) << std::left << li.short_spec()
               << std::setw(widths_[2] + 4) << std::left << li.plugin_type()
               << std::left << li.path() << "\n";
        return {result.str(), is_duplicate};
      }
    };
  }
}

#endif /* art_Framework_Art_detail_md_summary_MetadataSummaryForModule_h */

// Local variables:
// mode: c++
// End:
