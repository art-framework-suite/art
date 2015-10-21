#ifndef art_Framework_Art_detail_MetadataSummaryForSource_h
#define art_Framework_Art_detail_MetadataSummaryForSource_h

#include "art/Framework/Art/detail/LibraryInfoCollection.h"
#include "art/Framework/Art/detail/MetadataSummary.h"
#include "art/Framework/Art/detail/PrintFormatting.h"

#include <iomanip>
#include <sstream>
#include <string>

namespace art {
  namespace detail {

    template <>
    class MetadataSummaryFor<suffix_type::source> : public MetadataSummary {
    public:

      MetadataSummaryFor(LibraryInfoCollection const& coll)
        : coll_{coll}
        , widths_{
            columnWidth(coll, &LibraryInfo::short_spec, "module_type"),
            columnWidth(coll, &LibraryInfo::provider  , "Provider"),
            columnWidth(coll, &LibraryInfo::path      , "Source location")
          }
      {}

    private:

      LibraryInfoCollection const& coll_;
      Widths widths_;

      Widths const& doWidths() const override { return widths_; }

      std::string doHeader() const override
      {
        std::ostringstream result;
        result << indent0()
               << std::setw(widths_[0]+4) << std::left << "module_type"
               << std::setw(widths_[1]+4) << std::left << "Provider"
               << std::left << "Source location";
        return result.str();
      }

      std::string doSummary(LibraryInfo const& li) const override
      {
        auto const count = std::count_if( coll_.cbegin(), coll_.cend(),
                                          LibraryInfoMatch{li.short_spec()} );
        auto const dupl  = std::string(3, (count != 1) ? '*' : ' ');

        std::ostringstream result;
        result << dupl
               << std::setw(widths_[0]+4) << std::left << li.short_spec()
               << std::setw(widths_[1]+4) << std::left << li.provider()
               << std::left << li.path()
               << "\n";
        return result.str();
      }

    };

  }
}


#endif

// Local variables:
// mode: c++
// End:
