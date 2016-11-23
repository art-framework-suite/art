#ifndef art_Framework_Art_detail_md_collector_MetadataCollectorForService_h
#define art_Framework_Art_detail_md_collector_MetadataCollectorForService_h

#include "art/Framework/Art/detail/MetadataCollector.h"
#include "art/Framework/Art/detail/MetadataRegexHelpers.h"
#include "art/Framework/Art/detail/PrintFormatting.h"
#include "art/Framework/Art/detail/PluginMetadata.h"
#include "art/Framework/Art/detail/ServiceNames.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Utilities/bold_fontify.h"

#include <regex>

namespace art {
  namespace detail {

    template<>
    class MetadataCollectorFor<suffix_type::service> : public MetadataCollector {
    public:

      PluginMetadata doCollect(LibraryInfo const& li, std::string const& prefix) const override
      {
        return { header_(li), details_(li), allowed_configuration_(li, prefix) };
      }

    private:
      std::string header_(LibraryInfo const& li) const
      {
        static ServiceNames const serviceNames;
        std::string const& printed_name = serviceNames.fclname(li.short_spec());
        std::ostringstream result;
        result << indent_1()  << "service : "
               << bold_fontify(printed_name)
               << "\n\n";
        return result.str();
      }

      std::string details_(LibraryInfo const& li) const
      {
        std::ostringstream result;
        result << indent__2() << "provider: " << li.provider() << "\n"
               << indent__2() << "source  : " << li.path()     << "\n"
               << indent__2() << "library : " << li.so_name()  << "\n\n";
        return result.str();
      }

      std::string allowed_configuration_(LibraryInfo const& li, std::string const& prefix) const
      {
        std::ostringstream result;
        result << indent_1() << "Allowed configuration\n"
               << indent_1() << "---------------------\n";

        std::string printedConfig {describe(li.allowed_config(), prefix)};
        replace_label(li.short_spec(), printedConfig);

        result << printedConfig;
        return result.str();
      }
    };

  }
}

#endif /* art_Framework_Art_detail_md_collector_MetadataCollectorForService_h */

// Local variables:
// mode: c++
// End:
