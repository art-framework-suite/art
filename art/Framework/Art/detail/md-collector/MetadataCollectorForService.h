#ifndef art_Framework_Art_detail_md_collector_MetadataCollectorForService_h
#define art_Framework_Art_detail_md_collector_MetadataCollectorForService_h

#include "art/Framework/Art/detail/MetadataCollector.h"
#include "art/Framework/Art/detail/MetadataRegexHelpers.h"
#include "art/Framework/Art/detail/PluginMetadata.h"
#include "art/Framework/Art/detail/PrintFormatting.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Utilities/bold_fontify.h"

#include <regex>
#include <string>

namespace art {
  namespace detail {

    template <>
    class MetadataCollectorFor<suffix_type::service>
      : public MetadataCollector {
    public:
      PluginMetadata
      doCollect(LibraryInfo const& li, std::string const& prefix) const override
      {
        return {header_(li), details_(li), allowed_configuration_(li, prefix)};
      }

    private:
      std::string
      header_(LibraryInfo const& li) const
      {
        std::string const& printed_name = li.short_spec();
        std::ostringstream result;
        result << indent_1() << "service : " << bold_fontify(printed_name)
               << "\n\n";
        return result.str();
      }

      std::string
      details_(LibraryInfo const& li) const
      {
        std::ostringstream result;
        result << indent__2() << "provider: " << li.provider() << "\n"
               << indent__2() << "source  : " << li.path() << "\n"
               << indent__2() << "library : " << li.so_name() << "\n\n";
        return result.str();
      }

      std::string
      allowed_configuration_(LibraryInfo const& li,
                             std::string const& prefix) const
      {
        std::ostringstream result;
        result << indent_1() << "Allowed configuration\n"
               << indent_1() << "---------------------\n";
        result << describe(li.allowed_config(), prefix);
        return result.str();
      }
    };

  } // namespace detail
} // namespace art

#endif /* art_Framework_Art_detail_md_collector_MetadataCollectorForService_h  \
        */

// Local variables:
// mode: c++
// End:
