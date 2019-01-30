#ifndef art_Framework_Art_detail_md_summary_MetadataSummaryForTool_h
#define art_Framework_Art_detail_md_summary_MetadataSummaryForTool_h

#include "art/Framework/Art/detail/LibraryInfoCollection.h"
#include "art/Framework/Art/detail/MetadataSummary.h"
#include "art/Framework/Art/detail/PrintFormatting.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace art::detail {

  template <>
  class MetadataSummaryFor<suffix_type::tool> : public MetadataSummary {
  public:
    MetadataSummaryFor(LibraryInfoCollection const& coll)
      : widths_{std::to_string(coll.size()).size(),
                columnWidth(coll, &LibraryInfo::short_spec, "tool_type"),
                columnWidth(coll, &LibraryInfo::plugin_type, "Type"),
                columnWidth(coll, &LibraryInfo::path, "Source location")}
    {}

  private:
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
      result << indent(widths_[0] + 2) << std::setw(widths_[1] + 4) << std::left
             << "tool_type" << std::setw(widths_[2] + 4) << std::left << "Type"
             << std::left << "Source location";
      return result.str();
    }

    Summary
    doSummary(LibraryInfo const& li, std::size_t const entry) const override
    {
      std::ostringstream result;
      result << std::setw(widths_[0]) << std::right << entry << ". "
             << std::setw(widths_[1] + 4) << std::left << li.short_spec()
             << std::setw(widths_[2] + 4) << std::left << li.plugin_type()
             << std::left << li.path() << "\n";
      return {result.str(), false};
    }
  };

} // namespace art::detail

#endif /* art_Framework_Art_detail_md_summary_MetadataSummaryForTool_h */

// Local variables:
// mode: c++
// End:
